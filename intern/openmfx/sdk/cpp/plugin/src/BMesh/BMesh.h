#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <PluginSupport/MfxEffect>

namespace bmesh {

struct Vertex;
struct Edge;
struct Loop;
struct Face;

struct Vertex {
	Edge* edge;
	glm::vec3 position;
	int bufferIndexAttribute = -1;
	Vertex(){}
	Vertex(glm::vec3 _position) :position(_position) {}
};

struct Edge {
	Vertex* vertex[2];
	Edge* next[2]; // around each vertex
	Edge* prev[2];
	Loop* loop;

	int IndexOf(Vertex* v) const;
	Edge*& Next(Vertex* v);
	Edge*& Prev(Vertex* v);
};

struct Loop {
	Vertex* vertex;
	Edge* edge;
	Face* face;
	Loop* next;
	Loop* radialNext;
	Loop* prev;
	Loop* radialPrev;
	int bufferIndexAttribute = -1;
};

struct Face {
	int index;
	Loop *loop;
	int bufferIndexAttribute = -1;
	std::vector<Loop*> NeighborLoops() const;
	glm::vec3 normal();
};

struct BMesh {
	BMesh() {}
	~BMesh();
	BMesh(const BMesh&) = delete;
	BMesh& operator=(const BMesh&) = delete;
	void AddVertices(int count);
	Vertex* AddVertex();
	Vertex* AddVertex(glm::vec3 position);
	Vertex* AddVertex(float x, float y, float z);
	Edge* FindEdge(Vertex* v0, Vertex* v1);
	Edge* EnsureEdge(Vertex* v0, Vertex* v1);
	Edge* AddEdge(Vertex* v0, Vertex* v1);
	Face* AddFace(const std::vector<Vertex*>& faceVertices, int bufferIndexStartt = -1);
	Face* AddFace(const std::vector<int>& vertexIndices, int bufferIndexStart = -1);
	void RemoveVertex(Vertex* v, const bool& clearLooseEdge);
	void RemoveEdge(Edge* e, const bool& clearLooseEdge);
	void RemoveFace(Face* f, const bool &clearLooseEdge);

	/**
	 * May contain some nullptr in directions where there are no faces.
	 * In directions where there is more than one face, pick one arbitrarily.
	 */
	std::vector<Face*> NeighborFaces(Face *f) const;
	std::vector<Face*> NeighborFaces(Vertex* v) const;

	std::vector<Vertex*> NeighborVertices(Face* f) const;

	glm::vec3 Center(Face* f) const;


	// Conversion from and to buffers
	void initBMeshFromMfxProps(const MfxMeshProps& props, const MfxAttributeProps& point_buffer, const MfxAttributeProps& corner_buffer, const MfxAttributeProps& face_buffer);
	void toBuffer(MfxAttributeProps& point_buffer, MfxAttributeProps& corner_buffer, MfxAttributeProps& face_buffer);

	int vertexCount() const { return static_cast<int>(vertices.size()); }
	int edgeCount() const { return static_cast<int>(edges.size()); }
	int loopCount() const { return static_cast<int>(loops.size()); }
	int faceCount() const { return static_cast<int>(faces.size()); }

	std::vector<Vertex*> vertices;
	std::vector<Edge*> edges;
	std::vector<Loop*> loops;
	std::vector<Face*> faces;

private:
	Loop* AddLoop(Vertex* v, Edge* e, Face* f);
	void RemoveLoop(Loop* l, const bool& clearLooseEdge);
};

} // namespace bmesh
