#include "BMesh.h"
#include <algorithm> 

using glm::vec3;

namespace bmesh {

	int Edge::IndexOf(Vertex* v) const {
		if (vertex[0] == v) return 0;
		else if (vertex[1] == v) return 1;
		else return -1;
	}

	Edge*& Edge::Next(Vertex* v) {
		return next[IndexOf(v)];
	}

	Edge*& Edge::Prev(Vertex* v) {
		return prev[IndexOf(v)];
	}

	std::vector<Loop*> Face::NeighborLoops() const {
		std::vector<Loop*> neighbors;
		Loop* it = loop;
		do
		{
			neighbors.push_back(it);
			it = it->next;
		} while (it != loop);

		return neighbors;
	}
	
	glm::vec3 Face::normal() {
		vec3 result(0, 0, 0);
		std::vector<Loop*> faceLoops = NeighborLoops();
		int n = faceLoops.size();
		for (int i = 0; i < n; i++) {
			result += glm::cross(faceLoops[(i + 1) % n]->vertex->position - faceLoops[i]->vertex->position,
				faceLoops[(i - 1 + n) % n]->vertex->position - faceLoops[i]->vertex->position);
		}
		return glm::normalize(result);
	}


	BMesh::~BMesh() {
		for (auto v : vertices) delete v;
		for (auto e : edges) delete e;
		for (auto l : loops) delete l;
		for (auto f : faces) delete f;
	}

	void BMesh::AddVertices(int count) {
		vertices.reserve(vertices.size() + count);
		for (int i = 0; i < count; ++i) {
			AddVertex();
		}
	}

	Vertex* BMesh::AddVertex() {
		Vertex* v = new Vertex();
		v->edge = nullptr;
		vertices.push_back(v);
		return v;
	}

	Vertex* BMesh::AddVertex(vec3 position) {
		Vertex* v = new Vertex(position);
		v->edge = nullptr;
		vertices.push_back(v);
		return v;
	}

	Vertex* BMesh::AddVertex(float x, float y, float z) {
		return AddVertex(vec3(x, y, z));
	}

	Edge* BMesh::FindEdge(Vertex* v0, Vertex* v1) {
		Edge* e = v0->edge;
		if (e) {
			do {
				if (
					(e->vertex[0] == v0 && e->vertex[1] == v1) ||
					(e->vertex[0] == v1 && e->vertex[1] == v0)
					) return e;
				e = e->next[e->IndexOf(v0)];
			} while (e != v0->edge);
		}

		return nullptr;
	}

	Edge* BMesh::EnsureEdge(Vertex* v0, Vertex* v1) {
		Edge* e = FindEdge(v0, v1);
		if (e)
			return e;
		return AddEdge(v0, v1);
	}
	 
	Edge* BMesh::AddEdge(Vertex* v0, Vertex* v1) {
		Edge* e = new Edge();
		edges.push_back(e);
		e->vertex[0] = v0;
		e->vertex[1] = v1;
		e->loop = nullptr;
	
		// Insert
		if (v0->edge == nullptr) {
			v0->edge = e;
			e->next[0] = e;
			e->prev[0] = e;
		}
		else {
			e->next[0] = v0->edge->Next(v0);
			e->prev[0] = v0->edge;
			e->next[0]->Prev(v0) = e;
			e->prev[0]->Next(v0) = e;
		}

		if (v1->edge == nullptr) {
			v1->edge = e;
			e->next[1] = e;
			e->prev[1] = e;
		}
		else {
			e->next[1] = v1->edge->Next(v1);
			e->prev[1] = v1->edge;
			e->next[1]->Prev(v1) = e;
			e->prev[1]->Next(v1) = e;
		}

		return e;
	}

	Loop* BMesh::AddLoop(Vertex* v, Edge* e, Face* f) {
		Loop* l = new Loop();
		loops.push_back(l);

		l->vertex = v;
		l->edge = e;
		l->face = f;

		if (e->loop == nullptr)
		{
			e->loop = l;
			l->radialNext = l->radialPrev = l;
		}
		else
		{
			l->radialPrev = e->loop;
			l->radialNext = e->loop->radialNext;

			e->loop->radialNext->radialPrev = l;
			e->loop->radialNext = l;

			e->loop = l;
		}
		l->edge = e;

		if (f->loop == nullptr)
		{
			f->loop = l;
			l->next = l->prev = l;
		}
		else
		{
			l->prev = f->loop;
			l->next = f->loop->next;

			f->loop->next->prev = l;
			f->loop->next = l;

			f->loop = l;
		}

		return l;
	}

	Face* BMesh::AddFace(const std::vector<Vertex*>& faceVertices, int cornerbufferIndexStart) {
		int n = static_cast<int>(faceVertices.size());

		std::vector<Edge*> faceEdges(n);
		for (int i = 0; i < n; ++i) {
			faceEdges[i] = EnsureEdge(faceVertices[i], faceVertices[(i + 1) % n]);
		}

		Face* f = new Face();
		faces.push_back(f);
		f->index = static_cast<int>(faces.size() - 1);
		if (cornerbufferIndexStart != -1)
			f->bufferIndexAttribute = static_cast<int>(faces.size() - 1);
		f->loop = nullptr;

		for (int i = 0; i < n; ++i) {
			Loop* currentLoop = AddLoop(faceVertices[i], faceEdges[i], f);
			if (cornerbufferIndexStart != -1)
				currentLoop->bufferIndexAttribute = cornerbufferIndexStart + i;
		}

		f->loop = f->loop->next;

		return f;
	}

	Face* BMesh::AddFace(const std::vector<int>& vertexIndices, int cornerbufferIndexStart) {
		int n = static_cast<int>(vertexIndices.size());
		if (n == 0) return nullptr;
		std::vector<Vertex*> faceVertices(n);
		for (int i = 0; i < n; ++i) {
			faceVertices[i] = vertices[vertexIndices[i]];
		}
		return AddFace(faceVertices, cornerbufferIndexStart);
	}

	void BMesh::RemoveVertex(Vertex* v, const bool& clearLooseEdge) {
		while (v->edge != nullptr)
		{
			RemoveEdge(v->edge, clearLooseEdge);
		}
		auto it = std::find(vertices.begin(), vertices.end(), v);
		vertices.erase(it);
	}

	void BMesh::RemoveEdge(Edge* e, const bool& clearLooseEdge) {
		while (e->loop != nullptr)
		{
			RemoveLoop(e->loop, clearLooseEdge);
		}

		// Remove reference in vertices
		if (e == e->vertex[0]->edge) e->vertex[0]->edge = e->next[0] != e ? e->next[0] : nullptr;
		if (e == e->vertex[1]->edge) e->vertex[1]->edge = e->next[1] != e ? e->next[1] : nullptr;

		// Remove from linked lists
		e->prev[0]->Next(e->vertex[0]) = e->next[0];
		e->next[0]->Prev(e->vertex[0]) = e->prev[0];

		e->prev[1]->Next(e->vertex[1]) = e->next[1];
		e->next[1]->Prev(e->vertex[1]) = e->prev[1];

		auto it = std::find(edges.begin(), edges.end(), e);
		edges.erase(it);
	}

	void BMesh::RemoveLoop(Loop* l, const bool& clearLooseEdge) {
		if (l->face != nullptr) // null iff loop is called from RemoveFace
		{
			// Trigger removing other loops, and this one again with l.face == null
			RemoveFace(l->face, clearLooseEdge);
			return;
		}

		// remove from radial linked list
		if (l->radialNext == l)
		{
			l->edge->loop = nullptr;
			if  (clearLooseEdge)
				RemoveEdge(l->edge, clearLooseEdge);
		}
		else
		{
			l->radialPrev->radialNext = l->radialNext;
			l->radialNext->radialPrev = l->radialPrev;
			if (l->edge->loop == l)
			{
				l->edge->loop = l->radialNext;
			}
		}

		// forget other loops of the same face so thet they get released from memory
		l->next = nullptr;
		l->prev = nullptr;

		auto it = std::find(loops.begin(), loops.end(), l);
		loops.erase(it);
	}

	void BMesh::RemoveFace(Face* f, const bool& clearLooseEdge) {
		Loop* l = f->loop;
		Loop* nextL = nullptr;
		while (nextL != f->loop)
		{
			nextL = l->next;
			l->face = nullptr; // prevent infinite recursion, because otherwise RemoveLoop calls RemoveFace
			RemoveLoop(l, clearLooseEdge);
			l = nextL;
		}
		auto it = std::find(faces.begin(), faces.end(), f);
		faces.erase(it);
	}

	std::vector<Face*> BMesh::NeighborFaces(Face* f) const {
		std::vector<Face*> neighbors;
		Loop* it = f->loop;
		do
		{
			Face* nf = it->radialNext->face;
			if (nf != f) {
				neighbors.push_back(nf);
			}
			else {
				neighbors.push_back(nullptr);
			}

			it = it->next;
		} while (it != f->loop);

		return neighbors;
	}

	std::vector<Face*> BMesh::NeighborFaces(Vertex* v) const {
		std::vector<Face*> neighbors;
		Loop* it = v->edge->loop;
		do
		{
			neighbors.push_back(it->face);
			it = it->radialNext;
		} while (it != v->edge->loop);

		return neighbors;
	}

	std::vector<Vertex*> BMesh::NeighborVertices(Face* f) const {
		std::vector<Vertex*> neighbors;
		Loop* it = f->loop;
		do
		{
			neighbors.push_back(it->vertex);
			it = it->next;
		} while (it != f->loop);

		return neighbors;
	}

	vec3 BMesh::Center(Face* f) const {
		vec3 center(0);
		int count = 0;
		Loop* it = f->loop;
		do
		{
			center += it->vertex->position;
			++count;
			it = it->next;
		} while (it != f->loop);

		return center / static_cast<float>(count);
	}

	void BMesh::initBMeshFromMfxProps(const MfxMeshProps& props, const MfxAttributeProps& point_buffer, const MfxAttributeProps& corner_buffer, const MfxAttributeProps& face_buffer)
	{
		AddVertices(props.pointCount);
		for (int i = 0; i < props.pointCount; ++i) {
			float* in_p = reinterpret_cast<float*>(point_buffer.data + i * point_buffer.stride);
			vertices[i]->position = vec3(in_p[0], in_p[1], in_p[2]);
			vertices[i]->bufferIndexAttribute = i;
		}
		int currentCornerIndex = 0;
		for (int face = 0; face < props.faceCount; face++) {
			int faceSize = reinterpret_cast<int*>(face_buffer.data + face * face_buffer.stride)[0];
			std::vector<int> vertexIndices;
			for (int cornerIndex = 0; cornerIndex < faceSize; cornerIndex++) {
				vertexIndices.push_back(reinterpret_cast<int*>(corner_buffer.data + currentCornerIndex * corner_buffer.stride)[0]);
				currentCornerIndex++;
			}
			// Look for a different solution for loose edge
			AddFace(vertexIndices, currentCornerIndex - faceSize);
		}
	}

	void BMesh::toBuffer(MfxAttributeProps& point_buffer, MfxAttributeProps& corner_buffer, MfxAttributeProps& face_buffer) {
		for (int i = 0; i < vertexCount(); i++) {
			float* out_p = reinterpret_cast<float*>(point_buffer.data + i * point_buffer.stride);
			out_p[0] = vertices[i]->position.x;
			out_p[1] = vertices[i]->position.y;
			out_p[2] = vertices[i]->position.z;
			vertices[i]->bufferIndexAttribute = i;
		}
		int currentLoopIndex = 0;
		for (int faceIndex = 0; faceIndex < faceCount(); faceIndex++) {
			std::vector<Loop*> corners = faces[faceIndex]->NeighborLoops();
			int* face_out_p = reinterpret_cast<int*>(face_buffer.data + faceIndex * face_buffer.stride);
			face_out_p[0] = static_cast<int>(corners.size());
			for (int cornerFaceIndex = 0; cornerFaceIndex < face_out_p[0]; cornerFaceIndex++) {
				int* corner_out_p = reinterpret_cast<int*>(corner_buffer.data + currentLoopIndex * corner_buffer.stride);
				currentLoopIndex++;
				corner_out_p[0] = corners[cornerFaceIndex]->vertex->bufferIndexAttribute;
			}
		}
	}

} // namespace bmesh
