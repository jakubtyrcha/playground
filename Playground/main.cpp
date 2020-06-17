#include "gfx.h"
#include <fmt/format.h>
#include "array.h"

using namespace Container;

Array<char> GetFileContent(LPCWSTR path)
{
	HANDLE handle = CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
	if (handle == 0)
	{
		return {};
	}

	Array<char> result;

	DWORD bytes_read;
	char buffer[1024];
	while (ReadFile(handle, static_cast<void*>(buffer), 1024, &bytes_read, nullptr) && bytes_read)
	{
		result.Append(buffer, bytes_read);
	}

	CloseHandle(handle);

	return result;
}

struct PipelineState
{
};

int main(int argc, char** argv)
{
	Gfx::Device device;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};
	root_signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	root_signature_desc.Desc_1_1.NumParameters = 1;
	D3D12_ROOT_PARAMETER1 params[1] = {};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	D3D12_DESCRIPTOR_RANGE1 ranges[1] = {};
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	ranges[0].NumDescriptors = 1;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	params[0].DescriptorTable.NumDescriptorRanges = 1;
	params[0].DescriptorTable.pDescriptorRanges = ranges;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	root_signature_desc.Desc_1_1.pParameters = params;

	ID3DBlob* serialized_root_sig = nullptr;
	ID3DBlob* root_sig_errors = nullptr;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&root_signature_desc, &serialized_root_sig, &root_sig_errors);
	if (root_sig_errors)
	{
		fmt::print("RootSignature serialisation errors: {}", static_cast<const char*>(root_sig_errors->GetBufferPointer()));
	}

	if (!serialized_root_sig)
	{
		return 0;
	}

	ID3D12RootSignature* root_signature;
	hr = device.device_->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(), serialized_root_sig->GetBufferSize(), IID_PPV_ARGS(&root_signature));

	if (!root_signature)
	{
		return 0;
	}

	Array<char> shader = GetFileContent(L"../data/shader.dxil");

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_desc{};
	pipeline_desc.pRootSignature = root_signature;
	pipeline_desc.CS.pShaderBytecode = shader.Data();
	pipeline_desc.CS.BytecodeLength = shader.Size();

	ID3D12PipelineState* pso;
	hr = device.device_->CreateComputePipelineState(&pipeline_desc, IID_PPV_ARGS(&pso));

	if (!pso)
	{
		return 0;
	}

	// device.CreateComputeShader(L"../data/shader.dxil");

	pso->Release();
	root_signature->Release();

	return 0;
}
