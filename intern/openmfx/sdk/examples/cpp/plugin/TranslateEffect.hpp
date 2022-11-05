#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include <iostream>

/**
 * Translate the input geometry
 */
class TranslateEffect : public MfxEffect {
	protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);

		/*
		OfxParamSetHandle paramSet = nullptr;
		std::cout << "|| paramSet = " << paramSet << std::endl;
		std::cout << "|| &paramSet = " << &paramSet << std::endl;
		meshEffectSuite->getParamSet(descriptor, &paramSet);
		std::cout << "|| paramSet = " << paramSet << std::endl;

		OfxPropertySetHandle paramProps = nullptr;
		std::cout << "|| paramProps = " << paramProps << std::endl;
		std::cout << "|| &paramProps = " << &paramProps << std::endl;
		parameterSuite->paramDefine(paramSet, kOfxParamTypeInteger, "TEST", &paramProps);
		std::cout << "|| paramProps = " << paramProps << std::endl;

		propertySuite->propSetString(paramProps, kOfxPropLabel, 0, "TEST");
		std::cout << "|| paramProps = " << paramProps << std::endl;
		*/

		// Add a vector3 parameter
		AddParam("translation", double3{0.0, 0.0, 0.0})
		.Label("Translation") // Name used for display
		.Range(double3{-10.0, -10.0, -10.0}, double3{10.0, 10.0, 10.0});
		
		return kOfxStatOK;
	}
	
	OfxStatus Cook(OfxMeshEffectHandle) override {
		/*
		OfxMeshInputHandle input = nullptr;
		std::cout << "|| input = " << input << std::endl;
		std::cout << "|| &input = " << &input << std::endl;
		meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, nullptr);
		std::cout << "|| input = " << input << std::endl;
		OfxMeshHandle mesh = nullptr;
		std::cout << "|| mesh = " << mesh << std::endl;
		std::cout << "|| &mesh = " << &mesh << std::endl;
		meshEffectSuite->inputGetMesh(input, 0.0, &mesh, nullptr);
		std::cout << "|| mesh = " << mesh << std::endl;
		*/

		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxAttributeProps input_positions;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(input_positions);

		double3 translation = GetParam<double3>("translation").GetValue();

		MfxMeshProps input_mesh_props;
		input_mesh.FetchProperties(input_mesh_props);
		int output_point_count = input_mesh_props.pointCount;
		int output_corner_count = input_mesh_props.cornerCount;
		int output_face_count = input_mesh_props.faceCount;
		
		// Extra properties related to memory usage optimization
		int output_no_loose_edge = input_mesh_props.noLooseEdge;
		int output_constant_face_count = input_mesh_props.constantFaceSize;

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();
		
		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));
		
		if (output_constant_face_count < 0) {
			output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
				.ForwardFrom(input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize));
		}
		
		output_mesh.Allocate(
			output_point_count,
			output_corner_count,
			output_face_count,
			output_no_loose_edge,
			output_constant_face_count);

		MfxAttributeProps output_positions;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(output_positions);
		
		// (NB: This can totally benefit from parallelization using e.g. OpenMP)
		float tx = static_cast<float>(translation[0]);
		float ty = static_cast<float>(translation[1]);
		float tz = static_cast<float>(translation[2]);
		for (int i = 0 ; i < output_point_count ; ++i) {
			float *in_p = input_positions.at<float>(i);
			float *out_p = output_positions.at<float>(i);
			out_p[0] = in_p[0] + tx;
			out_p[1] = in_p[1] + ty;
			out_p[2] = in_p[2] + tz;
		}

		output_mesh.Release();
		input_mesh.Release();

		return kOfxStatOK;

	}
	public:
	const char* GetName() override {
		return "Translate";
	}

};