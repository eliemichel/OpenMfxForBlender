/*
 * Copyright 2019-2022 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <OpenMfx/Sdk/Cpp/Host/Host>
#include <OpenMfx/Sdk/Cpp/Host/MeshEffect>
#include <OpenMfx/Sdk/Cpp/Host/EffectRegistry>
#include <OpenMfx/Sdk/Cpp/Host/EffectLibrary>
#include <OpenMfx/Sdk/Cpp/Host/MeshProps>
#include <OpenMfx/Sdk/Cpp/Host/AttributeProps>

#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

/**
 * To illustrate this demo, we use an arbitrary mesh structure, here a very
 * simple one, that can only handle triangle meshes.
 */
struct MyMeshStruct {
	std::vector<std::array<float, 3>> points;
	std::vector<std::array<int, 3>> triangles;
	bool isInput = true;
};

/**
 * We subclass the OpenMfx::MfxHost base class in order to tell how to expose
 * our custom MyMeshStruct structure as an OpenMfx mesh to the effect, and how
 * to read back.
 */
class MyHost : public OpenMfx::Host {
	// Callback triggered at the beginning of meshEffectSuite->inputGetMesh()
	OfxStatus BeforeMeshGet(OpenMfx::Mesh* mfxMesh) override;
	// Callback triggered at the beginning of meshEffectSuite->inputReleaseMesh()
	OfxStatus BeforeMeshRelease(OpenMfx::Mesh* mfxMesh) override;
	// (The callbacks are defined bellow, after the main function)
};

// Just get an hardcoded mesh for the sake of the example.
const MyMeshStruct& getSomeMesh();

void run(const char* filepath) {
	// We first create the main host object. It must outlive any other OpenMfx
	// related operation so it is usually a singleton used throughout the whole
	// application.
	MyHost host;
	// (Optional: We alias some host functions to be able to easily use the
	// OpenMfx API from within this function.)
	const auto& propertySuite = host.propertySuite;

	// We then get the global registry. This is a singleton that avoid loading
	// multiple times the same EffectLibrary when multiple parts of the program
	// want to use effects from the same .ofx file.
	auto& registry = OpenMfx::EffectRegistry::GetInstance();
	// Before using the registry, we must connect it to our host.
	registry.setHost(&host);

	// We can now load an effect library (or retrieve it if it was already loaded)
	// if the file does not exist, or is not an OpenMfx library, this returns
	// a null pointer.
	// NB: It is important to call releaseLibrary once we no longer use it.
	OpenMfx::EffectLibrary* library = registry.getLibrary(filepath);
	if (nullptr == library) {
		throw std::runtime_error("Could not load binary!");
	}

	// An OpenFX library may contain both Mesh effects and Image effects, so it is
	// possible that effectCount (the number of Mesh effects found) is zero.
	if (library->effectCount() == 0) {
		throw std::runtime_error("No Mesh Effect found in library!");
	}

	// At this point, the effect's load() action has still not been called, but
	// we can list its identifier and version number. This can be interesting to
	// quickly list all available effects at startup.
	std::cout << "Using plugin '" << library->effectIdentifier(0) << "'" << std::endl;

	// The effect registry handles the loading and description of the effect for us
	// (and remembers it from one use of the effect to another one, there is no
	// need to call the describe action more than once for the whole application).
	OpenMfx::MeshEffect* effectDescriptor = registry.getEffectDescriptor(library, 0);
	if (!effectDescriptor) {
		throw std::runtime_error("Could not load effect!");
	}

	// For each use of the effect, we instantiate it. An instance is allowed to
	// retain "memory" from previous calls to the main cook function, which is why
	// you may want to use multiple instances of the same effect. It also stores
	// the value of its input meshes and parameters.
	OpenMfx::MeshEffect* effectInstance;
	if (!host.CreateInstance(effectDescriptor, effectInstance)) {
		throw std::runtime_error("Could not create instance!");
	}

	// Create the mesh structures for input/output data
	// We mark them with a boolean for BeforeMeshGet to know which one is which.
	MyMeshStruct inputMeshData = getSomeMesh();
	inputMeshData.isInput = true;
	MyMeshStruct outputMeshData;
	outputMeshData.isInput = false;

	// Set the inputs meshes of the effect instance
	// NB: the structure where the output mesh is stored is also considered as an
	// "input" in OpenMfx' vocable (maybe not the best idea ever but it behaves so
	// similarily all the time there was no clear reason to give it a different
	// name and duplicate all the functions). An effect has at most one output,
	// which is always called kOfxMeshMainOutput.
	int inputCount = effectInstance->inputs.count();
	std::cout << "Found " << inputCount << " inputs:" << std::endl;
	for (int i = 0; i < inputCount; ++i) {
		auto& input = effectInstance->inputs[i];
		std::cout << " - " << input.name() << std::endl;
		if (input.name() != kOfxMeshMainOutput) {
			// We provide a pointer to our own data structure but do convert to OpenMfx
			// yet because inputGetMesh might not be called. We will do it in
			// BeforeMeshGet instead.
			propertySuite->propSetPointer(&input.mesh.properties, kOfxMeshPropInternalData, 0, (void*)&inputMeshData);
		}
		else {
			propertySuite->propSetPointer(&input.mesh.properties, kOfxMeshPropInternalData, 0, (void*)&outputMeshData);
		}
	}

	// Set the parameters of the effect instance
	int paramCount = effectInstance->parameters.count();
	std::cout << "Found " << paramCount << " parameters:" << std::endl;
	for (int i = 0; i < paramCount; ++i) {
		const auto& param = effectInstance->parameters[i];
		std::cout << " - " << param.name << std::endl;
	}

	// Call the main function, which computes the output of the effect, given its
	// input mesh(es) and parameters. The cook action can be called many times.
	host.Cook(effectInstance);

	std::cout << "Output mesh has:" << std::endl;
	std::cout << " - " << outputMeshData.points.size() << " points" << std::endl;
	std::cout << " - " << outputMeshData.triangles.size() << " triangles" << std::endl;

	// Once done with the instance, we must destroy it.
	host.DestroyInstance(effectInstance);

	// Release the library, this means we will no longer use the effectDescriptor
	// and if the registry figures out that nobody is using the library any more,
	// the descriptor is destroyed and the library unloaded.
	registry.releaseLibrary(library);
}

int main(int argc, char** argv) {
	const char* filepath = "../../c/plugins/Debug/OpenMfx_Example_C_Plugin_mirror.ofx";
	if (argc > 1) filepath = argv[1];

	try {
		run(filepath);
	}
	catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


OfxStatus MyHost::BeforeMeshGet(OpenMfx::Mesh* mfxMesh) {
	MyMeshStruct* myMesh = nullptr;
	propertySuite->propGetPointer(&mfxMesh->properties, kOfxMeshPropInternalData, 0, (void**)&myMesh);

	if (myMesh == nullptr)
		return kOfxStatErrFatal;

	if (!myMesh->isInput)
		return kOfxStatOK;

	OpenMfx::MeshProps props;
	props.pointCount = static_cast<int>(myMesh->points.size());
	props.cornerCount = 3 * static_cast<int>(myMesh->triangles.size());
	props.faceCount = static_cast<int>(myMesh->triangles.size());
	props.constantFaceSize = 3;
	props.noLooseEdge = true;
	props.setProperties(propertySuite , &mfxMesh->properties);

	// Convert to attributes
	int attributeCount = mfxMesh->attributes.count();
	for (int i = 0; i < attributeCount; ++i) {
		auto& attribute = mfxMesh->attributes[i];
		if (attribute.attachment() == OpenMfx::AttributeAttachment::Point && attribute.name() == kOfxMeshAttribPointPosition) {
			attribute.setComponentCount(3);
			attribute.setType(OpenMfx::AttributeType::Float);
			attribute.setData((void*)myMesh->points.data());
			attribute.setByteStride(3 * sizeof(float));
		}
		else if (attribute.attachment() == OpenMfx::AttributeAttachment::Corner && attribute.name() == kOfxMeshAttribCornerPoint) {
			attribute.setComponentCount(1);
			attribute.setType(OpenMfx::AttributeType::Int);
			attribute.setData((void*)myMesh->triangles.data());
			attribute.setByteStride(3 * sizeof(float));
		}
		else if (attribute.attachment() == OpenMfx::AttributeAttachment::Face && attribute.name() == kOfxMeshAttribFaceSize) {
			attribute.setComponentCount(1);
			attribute.setType(OpenMfx::AttributeType::Int);
			attribute.setData(nullptr); // we use kOfxMeshPropConstantFaceSize instead
			attribute.setByteStride(0);
		}
	}

	return kOfxStatOK;
}

OfxStatus MyHost::BeforeMeshRelease(OpenMfx::Mesh* mfxMesh) {
	MyMeshStruct* myMesh = nullptr;
	propertySuite->propGetPointer(&mfxMesh->properties, kOfxMeshPropInternalData, 0, (void**)&myMesh);

	if (myMesh == nullptr)
		return kOfxStatErrFatal;

	if (myMesh->isInput)
		return kOfxStatOK;

	OpenMfx::MeshProps props;
	props.fetchProperties(propertySuite, &mfxMesh->properties);

	if (props.constantFaceSize != 3)
		return kOfxStatErrUnsupported;

	myMesh->points.resize(props.pointCount);
	myMesh->triangles.resize(props.faceCount);

	// Convert from attributes
	int attributeCount = mfxMesh->attributes.count();
	OpenMfx::AttributeProps attributeProps;
	for (int i = 0; i < attributeCount; ++i) {
		auto& attribute = mfxMesh->attributes[i];
		attributeProps.fetchProperties(propertySuite, &attribute.properties);
		if (attribute.attachment() == OpenMfx::AttributeAttachment::Point && attribute.name() == kOfxMeshAttribPointPosition) {
			if (attributeProps.type != OpenMfx::AttributeType::Float)
				return kOfxStatErrUnsupported;
			for (int j = 0; j < props.pointCount; ++j) {
				float* P = attributeProps.at<float>(j);
				for (int k = 0; k < 3; ++k) {
					myMesh->points[j][k] = P[k];
				}
			}
		}
		else if (attribute.attachment() == OpenMfx::AttributeAttachment::Corner && attribute.name() == kOfxMeshAttribCornerPoint) {
			if (attributeProps.type != OpenMfx::AttributeType::Int)
				return kOfxStatErrUnsupported;
			for (int j = 0; j < props.faceCount; ++j) {
				for (int k = 0; k < 3; ++k) {
					int C = *attributeProps.at<int>(3 * j + k);
					myMesh->triangles[j][k] = C;
				}
			}
		}
	}

	return kOfxStatOK;
}

const MyMeshStruct& getSomeMesh() {
	static MyMeshStruct inputMeshData = {
		{
			{-1.0, -1.0, -1.0},
			{1.0, -1.0, -1.0},
			{-1.0, 1.0, -1.0},
			{1.0, 1.0, -1.0},
			{-1.0, -1.0, 1.0},
			{1.0, -1.0, 1.0},
			{-1.0, 1.0, 1.0},
			{1.0, 1.0, 1.0},
		},
		{
			{0, 2, 3}, {0, 3, 1},
			{4, 5, 7}, {4, 7, 6},
			{0, 1, 5}, {0, 5, 4},
			{1, 3, 7}, {1, 7, 5},
			{3, 2, 6}, {3, 6, 7},
			{2, 0, 4}, {2, 4, 6},
		},
	};
	return inputMeshData;
}
