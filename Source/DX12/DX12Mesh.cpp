#include "DX12Mesh.h"

#include "CDX12Common.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>


CDX12Mesh::CDX12Mesh(const CDX12Mesh& other) : CDX12Mesh(other.mEngine, other.mFileName, other.hasTangents) {}


CDX12Mesh::CDX12Mesh(CDX12Engine* engine, std::string fileName, bool requireTangents)
{
	mEngine = engine;

	fileName = mEngine->GetMediaFolder() + fileName;

	mFileName = fileName;
	
	hasTangents = requireTangents;

	Assimp::Importer importer;

	// Flags for processing the mesh. Assimp provides a huge amount of control - right click any of these
	// and "Peek Definition" to see documention above each constant
	unsigned int assimpFlags = aiProcess_MakeLeftHanded |
		aiProcess_GenSmoothNormals |
		aiProcess_FixInfacingNormals |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_FindInvalidData |
		aiProcess_OptimizeMeshes |
		aiProcess_FindInstances |
		aiProcess_FindDegenerates |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_Debone |
		aiProcess_SplitByBoneCount |
		aiProcess_LimitBoneWeights |
		aiProcess_RemoveComponent;

	// Flags to specify what mesh data to ignore
	auto removeComponents = aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_TEXTURES | aiComponent_COLORS |
		aiComponent_ANIMATIONS | aiComponent_MATERIALS;

	// Add / remove tangents as required by user
	if (hasTangents)
	{
		assimpFlags |= aiProcess_CalcTangentSpace;
	}
	else
	{
		removeComponents |= aiComponent_TANGENTS_AND_BITANGENTS;
	}

	// Other miscellaneous settings
	importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f); // Smoothing angle for normals
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);  // Remove points and lines (keep triangles only)
	importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);                 // Remove degenerate triangles
	importer.SetPropertyBool(AI_CONFIG_PP_DB_ALL_OR_NONE, true);            // Default to removing bones/weights from meshes that don't need skinning

	// Set maximum bones that can affect one vertex, and also maximum bones affecting a single mesh
	unsigned int maxBonesPerVertex = 4; // The shaders support 4 bones per verted (null bones are added if necessary)
	unsigned int maxBonesPerMesh = 256; // Bone indexes are stored in a byte, so no more than 256
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, maxBonesPerVertex);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, maxBonesPerMesh);

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);

	// Import mesh with assimp given above requirements - log output
	Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE);
	auto scene = importer.ReadFile(fileName, assimpFlags);
	Assimp::DefaultLogger::kill();
	if (scene == nullptr)  throw std::runtime_error("Error loading mesh (" + fileName + "). " + importer.GetErrorString());
	if (scene->mNumMeshes == 0)  throw std::runtime_error("No usable geometry in mesh: " + fileName);

	//-----------------------------------

	//*********************************************************************//
	// Read node hierachy - each node has a matrix and contains sub-meshes //

	// Uses recursive helper functions to build node hierarchy
	mNodes.resize(CountNodes(scene->mRootNode));
	ReadNodes(scene->mRootNode, 0, 0);

	//******************************************//
	// Read geometry - multiple parts supported //

	mHasBones = false;
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
		if (scene->mMeshes[m]->HasBones())  mHasBones = true;

	// A mesh is made of sub-meshes, each one can have a different material (texture)
	// Import each sub-mesh in the file to seperate index / vertex buffer (could share buffers between sub-meshes but that would make things more complex)
	mSubMeshes.resize(scene->mNumMeshes);
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		auto assimpMesh = scene->mMeshes[m];
		std::string subMeshName = assimpMesh->mName.C_Str();
		auto& subMesh = mSubMeshes[m]; // Short name for the submesh we're currently preparing - makes code below more readable

		//-----------------------------------

		// Check for presence of position and normal data. Tangents and UVs are optional.
		std::vector<D3D12_INPUT_ELEMENT_DESC> vertexElements;
		unsigned int offset = 0;

		if (!assimpMesh->HasPositions()) 
			throw std::runtime_error("No position data for sub-mesh " + subMeshName + " in " + fileName);
		auto positionOffset = offset;
		vertexElements.push_back({ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, positionOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += 12;

		if (!assimpMesh->HasNormals()) 
			throw std::runtime_error("No normal data for sub-mesh " + subMeshName + " in " + fileName);
		auto normalOffset = offset;
		vertexElements.push_back({ "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += 12;

		auto tangentOffset = offset;
		if (hasTangents)
		{
			if (!assimpMesh->HasTangentsAndBitangents())  throw std::runtime_error("No tangent data for sub-mesh " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 12;
		}

		auto uvOffset = offset;
		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			if (assimpMesh->mNumUVComponents[0] != 2)  throw std::runtime_error("Unsupported texture coordinates in " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uvOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 8;
		}

		auto bonesOffset = offset;
		if (mHasBones)
		{
			vertexElements.push_back({ "bones"  , 0, DXGI_FORMAT_R8G8B8A8_UINT,      0, bonesOffset,     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 4;
			vertexElements.push_back({ "weights", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, bonesOffset + 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 16;
		}

		subMesh.vertexSize = offset;
		

		//-----------------------------------

		// Create CPU-side buffers to hold current mesh data - exact content is flexible so can't use a structure for a vertex - so just a block of bytes
		// Note: for large arrays a unique_ptr is better than a vector because vectors default-initialise all the values which is a waste of time.
		subMesh.numVertices = assimpMesh->mNumVertices;
		subMesh.numIndices = assimpMesh->mNumFaces * 3;
		auto vertices = std::make_unique<unsigned char[]>(subMesh.numVertices * subMesh.vertexSize);
		auto indices = std::make_unique<unsigned char[]>(subMesh.numIndices * 4); // Using 32 bit indexes (4 bytes) for each indeex

		//-----------------------------------

		// Copy mesh data from assimp to our CPU-side vertex buffer

		auto assimpPosition = reinterpret_cast<CVector3*>(assimpMesh->mVertices);
		auto position = vertices.get() + positionOffset;
		auto positionEnd = position + subMesh.numVertices * subMesh.vertexSize;
		while (position != positionEnd)
		{
			*(CVector3*)position = *assimpPosition;
			position += subMesh.vertexSize;
			++assimpPosition;
		}

		auto assimpNormal = reinterpret_cast<CVector3*>(assimpMesh->mNormals);
		auto normal = vertices.get() + normalOffset;
		auto normalEnd = normal + subMesh.numVertices * subMesh.vertexSize;
		while (normal != normalEnd)
		{
			*(CVector3*)normal = *assimpNormal;
			normal += subMesh.vertexSize;
			++assimpNormal;
		}

		if (hasTangents)
		{
			auto assimpTangent = reinterpret_cast<CVector3*>(assimpMesh->mTangents);
			auto tangent = vertices.get() + tangentOffset;
			auto tangentEnd = tangent + subMesh.numVertices * subMesh.vertexSize;
			while (tangent != tangentEnd)
			{
				*(CVector3*)tangent = *assimpTangent;
				tangent += subMesh.vertexSize;
				++assimpTangent;
			}
		}

		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			auto assimpUV = assimpMesh->mTextureCoords[0];
			auto uv = vertices.get() + uvOffset;
			auto uvEnd = uv + subMesh.numVertices * subMesh.vertexSize;
			while (uv != uvEnd)
			{
				*(CVector2*)uv = CVector2(assimpUV->x, assimpUV->y);
				uv += subMesh.vertexSize;
				++assimpUV;
			}
		}

		if (mHasBones)
		{
			if (assimpMesh->HasBones())
			{
				// Set all bones and weights to 0 to start with
				auto bones = vertices.get() + bonesOffset;
				auto bonesEnd = bones + subMesh.numVertices * subMesh.vertexSize;
				while (bones != bonesEnd)
				{
					memset(bones, 0, 20);
					bones += subMesh.vertexSize;
				}

				for (auto& node : mNodes)
				{
					node.offsetMatrix = MatrixIdentity();
				}

				// Go through each assimp bone
				bones = vertices.get() + bonesOffset;
				for (unsigned int i = 0; i < assimpMesh->mNumBones; ++i)
				{
					// Get offset matrix for the bone (transform from skinned mesh root to bone root
					auto assimpBone = assimpMesh->mBones[i];
					std::string boneName = assimpBone->mName.C_Str();
					unsigned int nodeIndex;
					for (nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
					{
						if (mNodes[nodeIndex].name == boneName)
						{
							mNodes[nodeIndex].offsetMatrix.SetValues(&assimpBone->mOffsetMatrix.a1);
							mNodes[nodeIndex].offsetMatrix.Transpose(); // Assimp stores matrices differently to this app
							break;
						}
					}
					if (nodeIndex == mNodes.size())  throw std::runtime_error("Bone with no matching node in " + fileName);

					// Go through each weight of the bone and update the vertex it influences
					// Find the first 0 weight on that vertex and put the new influence / weight there.
					// A vertex can only have up to 4 influences
					for (unsigned int j = 0; j < assimpBone->mNumWeights; ++j)
					{
						auto vertexIndex = assimpBone->mWeights[j].mVertexId;
						auto bone = bones + vertexIndex * subMesh.vertexSize;
						auto weight = (float*)(bone + 4);
						auto lastWeight = weight + 3;
						while (*weight != 0.0f && weight != lastWeight)
						{
							bone++; weight++;
						}
						if (*weight == 0.0f)
						{
							*bone = nodeIndex;
							*weight = assimpBone->mWeights[j].mWeight;
						}
					}
				}
			}
			else
			{
				// In a mesh that uses skinning any sub-meshes that don't contain bones are given bones so the whole mesh can use one shader
				unsigned int subMeshNode = 0;
				for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
				{
					for (auto& subMeshIndex : mNodes[nodeIndex].subMeshes)
					{
						if (subMeshIndex == m)
							subMeshNode = nodeIndex;
					}
				}

				auto bones = vertices.get() + bonesOffset;
				auto bonesEnd = bones + subMesh.numVertices * subMesh.vertexSize;
				while (bones != bonesEnd)
				{
					memset(bones, 0, 20);
					bones[0] = subMeshNode;
					*(float*)(bones + 4) = 1.0f;
					bones += subMesh.vertexSize;
				}
			}
		}

		//-----------------------------------

		// Copy face data from assimp to our CPU-side index buffer
		if (!assimpMesh->HasFaces())  throw std::runtime_error("No face data in " + subMeshName + " in " + fileName);

		auto index = reinterpret_cast<DWORD*>(indices.get());
		for (unsigned int face = 0; face < assimpMesh->mNumFaces; ++face)
		{
			*index++ = assimpMesh->mFaces[face].mIndices[0];
			*index++ = assimpMesh->mFaces[face].mIndices[1];
			*index++ = assimpMesh->mFaces[face].mIndices[2];
		}

		//-----------------------------------

		 // Create a root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(engine->mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		// A single 32-bit constant root parameter that is used by the vertex shader.
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		// Serialize the root signature.
		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
			featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
		// Create the root signature.
		(engine->mDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		} pipelineStateStream;



		auto vertexShaderBlob = engine->mPbrVertexShader.mblob;
		auto pixelShaderBlob = engine->mPbrPixelShader.mblob;

		if(hasTangents)
		{
			vertexShaderBlob = engine->mPbrNormalVertexShader.mblob;
			pixelShaderBlob = engine->mPbrNormalPixelShader.mblob;
		}

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1;
		rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		pipelineStateStream.pRootSignature = mRootSignature.Get();
		pipelineStateStream.InputLayout = { vertexElements.data(), sizeof(vertexElements) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.RTVFormats = rtvFormats;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(PipelineStateStream), &pipelineStateStream
		};
		auto hr = engine->mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState));
		
		// Execute command list

		engine->mCommandList->Close();

		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		engine->mCommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);

		ID3D12CommandList* const ppCommandLists[] = {
			engine->mCommandList.Get()
		};

		engine->mCommandQueue->ExecuteCommandLists(1, ppCommandLists);
		uint64_t fenceValue = engine->Signal();

		engine->mCommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
		engine->mCommandListQueue.push(engine->mCommandList.Get());

		// The ownership of the command allocator has been transferred to the ComPtr
		// in the command allocator queue. It is safe to release the reference 
		// in this temporary COM pointer here.
		commandAllocator->Release();

		if (!(engine->mFence->GetCompletedValue() >= fenceValue))
		{
			engine->mFence->SetEventOnCompletion(fenceValue, engine->mFenceEvent);
			::WaitForSingleObject(engine->mFenceEvent, DWORD_MAX);
		}
	}
}

void CDX12Mesh::Render(std::vector<CMatrix4x4>& modelMatrices)
{
	// Skinning needs all matrices available in the shader at the same time, so first calculate all the absolute
	// matrices before rendering anything
	std::vector<CMatrix4x4> absoluteMatrices(modelMatrices.size());
	absoluteMatrices[0] = modelMatrices[0]; // First matrix for a model is the root matrix, already in world space
	for (unsigned int nodeIndex = 1; nodeIndex < mNodes.size(); ++nodeIndex)
	{
		// Multiply each model matrix by its parent's absolute world matrix (already calculated earlier in this loop)
		// Same process as for rigid bodies, simply done prior to rendering now
		absoluteMatrices[nodeIndex] = modelMatrices[nodeIndex] * absoluteMatrices[mNodes[nodeIndex].parentIndex];
	}

	if (mHasBones) // Render a mesh that uses skinning
	{
		// Advanced point: the above loop will get the absolute world matrices **of the bones**. However, they are
		// not actually rendered, they merely influence the skinned mesh, which has its origin at a particular node.
		// So for each bone there is a fixed offset (transform) between where that bone is and where the root of the
		// skinned mesh is. We need to apply that offset to each of the bone matrices calculated in the last loop to make
		// the bone influences work on the skinned mesh.
		// These offset matrices are fixed for the model and have been calculated when the mesh was imported
		for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
		{
			absoluteMatrices[nodeIndex] = mNodes[nodeIndex].offsetMatrix * absoluteMatrices[nodeIndex];
		}

	//	// Send all matrices over to the GPU for skinning via a constant buffer - each matrix can represent a bone which influences nearby vertices
	//	// MISSING - code to fill the gPerModelConstants.boneMatrices array with the contents of the absoluteMatrices vector
	//	for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
	//	{
	//		gPerModelConstants.boneMatrices[nodeIndex] = absoluteMatrices[nodeIndex];
	//	}

	//	mEngine->UpdateModelConstantBuffer(gPerModelConstantBuffer.Get(), gPerModelConstants); // Send to GPU

	//	// Indicate that the constant buffer we just updated is for use in the vertex shader (VS), geometry shader (GS) and pixel shader (PS)
	//	mEngine->GetContext()->VSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf()); // First parameter must match constant buffer number in the shader
	//	mEngine->GetContext()->GSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf()); // First parameter must match constant buffer number in the shader
	//	mEngine->GetContext()->PSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf());

	//	// Already sent over all the absolute matrices for the entire mesh so we can render sub-meshes directly
	//	// rather than iterating through the nodes.
	//	for (auto& subMesh : mSubMeshes)
	//	{
	//		RenderSubMesh(subMesh);
	//	}
	//}
	//else
	//{
	//	// Render a mesh without skinning. Although slightly reorganised to use the matrices calculated
	//	// above, this is basically the same code as the rigid body animation lab
	//	// Iterate through each node
	//	for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
	//	{
	//		// Send this node's matrix to the GPU via a constant buffer
	//		gPerModelConstants.worldMatrix = absoluteMatrices[nodeIndex];
	//		mEngine->UpdateModelConstantBuffer(gPerModelConstantBuffer.Get(), gPerModelConstants); // Send to GPU

	//		// Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
	//		mEngine->GetContext()->VSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf()); // First parameter must match constant buffer number in the shader
	//		mEngine->GetContext()->GSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf());
	//		mEngine->GetContext()->PSSetConstantBuffers(0, 1, gPerModelConstantBuffer.GetAddressOf());

	//		// Render the sub-meshes attached to this node (no bones - rigid movement)
	//		for (auto& subMeshIndex : mNodes[nodeIndex].subMeshes)
	//		{
	//			RenderSubMesh(mSubMeshes[subMeshIndex]);
	//		}
	//	}
		
	}
}

unsigned CDX12Mesh::CountNodes(aiNode* assimpNode)
{
	unsigned int count = 1;
	for (unsigned int child = 0; child < assimpNode->mNumChildren; ++child)
		count += CountNodes(assimpNode->mChildren[child]);
	return count;
}

unsigned int CDX12Mesh::ReadNodes(aiNode* assimpNode, unsigned int nodeIndex, unsigned int parentIndex)
{
	auto& node = mNodes[nodeIndex];
	node.parentIndex = parentIndex;
	const auto thisIndex = nodeIndex;
	++nodeIndex;

	node.name = assimpNode->mName.C_Str();

	node.defaultMatrix.SetValues(&assimpNode->mTransformation.a1);
	node.defaultMatrix.Transpose(); // Assimp stores matrices differently to this app

	node.subMeshes.resize(assimpNode->mNumMeshes);
	for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
	{
		node.subMeshes[i] = assimpNode->mMeshes[i];
	}

	node.childNodes.resize(assimpNode->mNumChildren);
	for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
	{
		node.childNodes[i] = nodeIndex;
		nodeIndex = ReadNodes(assimpNode->mChildren[i], nodeIndex, thisIndex);
	}

	return nodeIndex;
}

void CDX12Mesh::RenderSubMesh(const SubMesh& subMesh) const
{
	//// Set vertex buffer as next data source for GPU
	mEngine->mCommandList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView);

	//// Set index buffer as next data source for GPU, indicate it uses 32-bit integers
	mEngine->mCommandList->IASetIndexBuffer(&subMesh.indexBufferView);

	//// Using triangle lists only in this class
	mEngine->mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// Render mesh
	mEngine->mCommandList->DrawIndexedInstanced(subMesh.numIndices, 1, 0, 0, 0);
}

