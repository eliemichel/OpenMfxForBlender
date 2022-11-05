#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include <OpenMfx/Sdk/Cpp/Plugin/utils>

static const float output_point_data[] = {
	-1, -1, -1,
	1, -1, -1,
	1, 1, -1,
	-1, 1, -1,

	-1, -1, 1,
	1, -1, 1,
	1, 1, 1,
	-1, 1, 1,
};

static const int output_vertex_data[] = {
	3, 2, 1, 0,
	4, 5, 6, 7,
	0, 1, 5, 4,
	2, 3, 7, 6,
	4, 7, 3, 0,
	1, 2, 6, 5,
};

/**
 * The Box Generator effects creates a cubic primitive
 */
class BoxGeneratorEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "BoxGenerator";
	}

protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainOutput);
		
		AddParam("size", double3{1.0, 1.0, 1.0})
			.Label("Size")
			.Range(double3{0.0, 0.0, 0.0}, double3{10.0, 10.0, 10.0});

		AddParam("position", double3{ 0.0, 0.0, 0.0 })
			.Label("Position")
			.Range(double3{ -10.0, -10.0, -10.0 }, double3{ 10.0, 10.0, 10.0 });
		
		return kOfxStatOK;
	}
	
	OfxStatus Cook(OfxMeshEffectHandle) override {

		auto size = to_float3(GetParam<double3>("size").GetValue());
		auto position = to_float3(GetParam<double3>("position").GetValue());

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();

		int output_point_count = 8;
		int output_face_count = 6;
		int output_corner_count = 4 * output_face_count;
		
		output_mesh.Allocate(
			output_point_count,
			output_corner_count,
			output_face_count,
			1 /* output_no_loose_edge */,
			-1 /* 4 output_constant_face_size */);

		MfxAttributeProps output_positions;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(output_positions);

		for (int i = 0; i < output_point_count; ++i) {
			float *p = output_positions.at<float>(i);
			for (int k = 0; k < 3; ++k) {
				p[k] = position[k] + size[k] * 0.5f * output_point_data[3 * i + k];
			}
		}

		MfxAttributeProps output_corners;
		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.FetchProperties(output_corners);

		for (int i = 0; i < output_corner_count; ++i) {
			*output_corners.at<int>(i) = output_vertex_data[i];
		}

		MfxAttributeProps output_faces;
		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
			.FetchProperties(output_faces);

		for (int i = 0; i < output_face_count; ++i) {
			*output_faces.at<int>(i) = 4;
		}

		output_mesh.Release();
		
		return kOfxStatOK;
	}
};