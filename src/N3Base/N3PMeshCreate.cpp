// N3PMeshCreate.cpp: implementation of the CN3PMeshCreate class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3PMeshCreate.h"
#include "N3PMesh.h"
#include "N3PMeshInstance.h"
#include "N3Mesh.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Maximum buffer sizes. For greater flexibility, the buffers should reallocate dynamically.
#define MAX_COLLAPSES             5000
#define MAX_INDEX_CHANGES         10000
#define MAX_VERTICES_PER_MATERIAL 10000

//////////////////////////////////////////////////////////////////////////////
//
// Helper functions

// swap two variables
template <class T>
void swap(T& a, T& b)
{
	T t;

	t = a;
	a = b;
	b = t;
}

// swap whole triangles
void CN3PMeshCreate::swap_triangle(uint16_t* t1, uint16_t* t2)
{
	swap(t1[0], t2[0]);
	swap(t1[1], t2[1]);
	swap(t1[2], t2[2]);
}

// 없어질 삼각형의 넓이 계산, 혹은 변의 길이의 합
float CN3PMeshCreate::GetTriCollapsesLoss(uint16_t* pTriIndex, bool bArea)
{
	// These are the corners of the triangle.
	__Vector3 pts[3] = { m_pVertices[pTriIndex[0]], m_pVertices[pTriIndex[1]],
		m_pVertices[pTriIndex[2]] };

	if (bArea)
	{
		// Calculate the area.
		__Vector3 cross, V1, V2;
		V1 = pts[2] - pts[0];
		V2 = pts[1] - pts[0];
		cross.Cross(V1, V2);
		return cross.Magnitude();
	}
	else
	{
		// Calculate side length.
		__Vector3 V1, V2, V3;
		V1          = pts[2] - pts[0];
		V2          = pts[1] - pts[0];
		V3          = pts[1] - pts[2];

		float fLoss = V1.Magnitude() + V2.Magnitude() + V3.Magnitude() + 0.0001f;
		__ASSERT(fLoss > 0, "Loss value is less than 0");
		return fLoss;
	}
}

// add the cost of a triangle being modified (ie one of its vertices being changed) into the accumulator "sofar".
// This is the meat of the edge evaluation function.
void CN3PMeshCreate::combine_modified(
	float& sofar, uint16_t* tri, int which, int what_to, bool bSumOfLoss)
{
	// These are the corners of the triangle at the moment.
	__Vector3 pts[3]   = { m_pVertices[tri[0]], m_pVertices[tri[1]], m_pVertices[tri[2]] };

	// This is a point in the plane of the triangle.
	__Vector3 in_plane = pts[0];

	// Calculate the area and face normal for the triangle at the moment.
	__Vector3 oldcross, V1, V2;
	V1 = pts[2] - pts[0];
	V2 = pts[1] - pts[0];
	oldcross.Cross(V1, V2);

	float oldarea = oldcross.Magnitude();
	__Vector3 oldnorm;
	oldnorm.Normalize(oldcross);

	// Change the triangle.
	pts[which] = m_pVertices[what_to];

	// Re-calculate the area and face normal.
	__Vector3 newcross;
	V1 = pts[2] - pts[0];
	V2 = pts[1] - pts[0];
	newcross.Cross(V1, V2);

	float newarea = newcross.Magnitude();
	__Vector3 newnorm;
	newnorm.Normalize(newcross);

	// A measure of the difference in the face normals.
	float cosangdiff            = newnorm.Dot(oldnorm);

	// Calculate some statistics about the triangle change.
	V1                          = m_pVertices[what_to] - in_plane;
	float volume_change         = std::abs(V1.Dot(oldcross));

	// The angle change weighted by the area of the triangle.
	float weighted_angle_change = (1.0f - cosangdiff) * (oldarea + newarea);

	// cosangdiff가 1보다 아주 조금 클때가 있어서 weighted_angle_change가 -값이 나올때가 있다.
	if (weighted_angle_change < 0.0f)
		weighted_angle_change = 0.0f;
	//__ASSERT(weighted_angle_change>=0.0f, "weighted_angle_change > 0 이어야 한다.");

	// These numbers are not in the same "units", one being length^3 and the other length^2
	// And let's put some arbitrary weighting on these things.
	volume_change         = powf(volume_change, .333333f);
	weighted_angle_change = powf(weighted_angle_change, .5f) * 5.f;

	// Work out a cost for the changing of this triangle
	float newval          = weighted_angle_change + volume_change;

	if (bSumOfLoss)
	{
		sofar += newval;
	}
	else
	{
		// And factor it in. Here I choose to accumulate the maximum cost.
		if (newval > sofar)
			sofar = newval;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3PMeshCreate::CN3PMeshCreate()
{
	m_pVertices        = nullptr;
	m_pIndices         = nullptr;

	m_pCollapses       = nullptr;
	m_pAllIndexChanges = nullptr;

	Release();
}

CN3PMeshCreate::~CN3PMeshCreate()
{
	CN3PMeshCreate::Release();
}

void CN3PMeshCreate::Release()
{
	if (m_pVertices)
	{
		delete[] m_pVertices;
		m_pVertices = nullptr;
	}
	if (m_pIndices)
	{
		delete[] m_pIndices;
		m_pIndices = nullptr;
	}

	if (m_pCollapses)
	{
		delete[] m_pCollapses;
		m_pCollapses = nullptr;
	}
	if (m_pAllIndexChanges)
	{
		delete[] m_pAllIndexChanges;
		m_pAllIndexChanges = nullptr;
	}

	m_pCollapseUpTo      = nullptr;
	m_iTotalIndexChanges = 0;
	m_iNumVertices       = 0;
	m_iNumIndices        = 0;
}

// collapse pt_from onto pt_to, adding the collapse to the collapse list
void CN3PMeshCreate::Collapse(uint16_t& pt_to, uint16_t& pt_from, float edge_val)
{
	// Swap the collapsing vertex to the end of the vertex list.
	SwapToEnd(pt_from, m_pCollapses, m_pCollapseUpTo, pt_to, pt_from);

	// Now we do the collapse.
	m_iNumVertices--;

	// Run through uncollapsed triangles to find ones that
	// refer to this vertex, and count up the ones that disappear.
	m_pCollapseUpTo->NumTrianglesToLose = 0;
	m_pCollapseUpTo->IndexChanges       = nullptr;
	m_pCollapseUpTo->NumIndicesToChange = 0;

	// step through the triangle list
	int tri_index;
	for (tri_index = 0; tri_index < m_iNumIndices;)
	{
		// step through each point in the triangle
		for (int pt = 0; pt < 3; pt++)
		{
			// Is this the collapsing vertex?
			if (m_pIndices[tri_index + pt] == m_iNumVertices)
			{
				// Yes, but will this triangle be degenerate? Must look for
				// the other end of the collapse edge
				int pt2;

				// look for the other point
				for (pt2 = 0; pt2 < 3; pt2++)
				{
					if (m_pIndices[tri_index + pt2] == pt_to)
					{
						// This triangle contains the collapsing edge.

						// It goes on the end of the list, unless it's already there.
						m_pCollapseUpTo->NumTrianglesToLose++;
						m_iNumIndices -= 3;

						if (tri_index == m_iNumIndices)
						{
							// This triangle is the end of the list. We're done here,
							// and we've got no more triangles to look at (because it's
							// the end of the list)
							goto done_triangle_list;
						}

						// Swap the triangles.
						swap_triangle(m_pIndices + tri_index, m_pIndices + m_iNumIndices);

						// And any references to them (in other collapses)
						for (__PMCEdgeCollapse* c = m_pCollapses; c < m_pCollapseUpTo; c++)
						{
							// Look through the index changes.
							int* ic;

							for (ic = c->IndexChanges; ic < c->IndexChanges + c->NumIndicesToChange;
								ic++)
							{
								if (*ic >= tri_index && *ic < tri_index + 3)
								{
									// This index change refers to the triangle that's
									// been moved!
									*ic += (m_iNumIndices - tri_index);
								}
								else if (*ic >= m_iNumIndices && *ic < m_iNumIndices + 3)
								{
									// This index change refers to the other triangle
									*ic -= (m_iNumIndices - tri_index);
								}
							}
						}

						// So, this triangle became redundant. Try the next triangle,
						// but don't increment tri_index.
						// 이번에 검사한 삼각형은 사라졌다. 리스트 맨뒤로 보냈으니
						// tri_index는 중가시키지 않는다.
						goto try_same_triangle_location;
					}
				}

				// 여기에 오면 이 삼각형은 사라지는 삼각형은 아니다. 그러나 한 점이 다른점으로 옮겨진다.
				// This triangle isn't degenerate if we've got here. But it does
				// have a reference to the collapsing vertex. This is fine - it can
				// now just refer to the non-collapsing vertex.

				// We want to record the index change. First, do we have a buffer of them?
				if (!m_pCollapseUpTo->IndexChanges)
				{
					// We haven't recorded an index change yet.
					m_pCollapseUpTo->IndexChanges = m_pAllIndexChanges + m_iTotalIndexChanges;
				}

				// Add another index change on.
				__ASSERT(m_iTotalIndexChanges < MAX_INDEX_CHANGES,
					"You must increase MAX_INDEX_CHANGES");

				m_pAllIndexChanges[m_iTotalIndexChanges++] = tri_index + pt;
				m_pCollapseUpTo->NumIndicesToChange++;

				// And actually do the change.
				m_pIndices[tri_index + pt] = pt_to;

				// Definately no more indices to change in this triangle.
				goto try_another_triangle;
			}
		}

	try_another_triangle:
		// step on to the next triangle
		tri_index += 3;

	try_same_triangle_location:;
		// You can jump to here if the next triangle is still in the same
		// place because triangles have been moved around.
	}

done_triangle_list:

	// So, the triangle list has been edited to account for the collapse.

	// Were any other vertices made redundant by the loss of these triangles?
	m_pCollapseUpTo->NumVerticesToLose = 1; // That's the collapser itself, of course.

	// Flag all vertices to see if they're referred to by triangles any more.
	int referred[MAX_VERTICES_PER_MATERIAL];
	memset(referred, 0, sizeof(referred));
	__ASSERT(
		m_iNumVertices < MAX_VERTICES_PER_MATERIAL, "You must increase MAX_VERTICES_PER_MATERIAL");

	int i;
	for (i = 0; i < m_iNumIndices; i++)
	{
		__ASSERT(m_pIndices[i] < m_iNumVertices, "indices array index overflow");
		referred[m_pIndices[i]] = 1;
	}

	// If not, let's lose them.
	for (i = 0; i < m_iNumVertices;)
	{
		if (!referred[i])
		{
			// We can swap this vertex with the one at the end of the list.
			SwapToEnd(i, m_pCollapses, m_pCollapseUpTo + 1, pt_to, pt_from);

			// And better swap its "referred" status.
			swap(referred[i], referred[m_iNumVertices - 1]);

			// One more vertex lost.
			m_pCollapseUpTo->NumVerticesToLose++;
			m_iNumVertices--;

			// And don't increment i
			continue;
		}

		i++;
	}

	// Fill in the remaining collapse data.
	m_pCollapseUpTo->CollapseTo = pt_to;
	m_pCollapseUpTo->Value      = edge_val;
	m_pCollapseUpTo++;
}

// Evaluate the cost of an edge, and if it's better than the best edge so far, record it
void CN3PMeshCreate::TryEdge(
	int pt_a, int pt_b, float& be_val, uint16_t& be_to, uint16_t& be_from, bool& IsOver)
{
	// There's actually two edge modifications being considered here:
	// - collapse of pt_a onto pt_b (a_loss)
	// - collapse of pt_b onto pt_a (b_loss)

	float a_loss = 0.f;
	float b_loss = 0.f;
	if (m_PMCOption.bUseSumOfLoss)
	{
		a_loss = 0.f;
		b_loss = 0.f;
	}
	else
	{
		// Start with both costing very little.
		a_loss = -1000000000000000.f;
		b_loss = -1000000000000000.f;
	}

	// Look for all triangles affected by the collapse
	for (uint16_t* tri = m_pIndices; tri < m_pIndices + m_iNumIndices; tri += 3)
	{
		if (tri[0] == pt_a)
		{
			if (tri[1] != pt_b && tri[2] != pt_b)
			{
				combine_modified(a_loss, tri, 0, pt_b, m_PMCOption.bUseSumOfLoss);
				__ASSERT(a_loss >= 0.0f, "loss > 0이어야 한다.");
			}
			else
			{
				// tri collapses.
				if (m_PMCOption.bTriangleWeight)
				{
					float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
								   * m_PMCOption.fWeight;
					if (m_PMCOption.bUseSumOfLoss)
					{
						a_loss += t_loss;
						b_loss += t_loss;
					}
					else
					{
						if (t_loss > a_loss)
							a_loss = t_loss;
					}
					__ASSERT(a_loss >= 0.0f && b_loss >= 0.0f, "loss > 0이어야 한다.");
				}
				continue;
			}
		}
		else if (tri[1] == pt_a)
		{
			if (tri[2] != pt_b && tri[0] != pt_b)
			{
				combine_modified(a_loss, tri, 1, pt_b, m_PMCOption.bUseSumOfLoss);
				__ASSERT(a_loss >= 0.0f, "loss > 0이어야 한다.");
			}
			else
			{
				// tri collapses.
				if (m_PMCOption.bTriangleWeight)
				{
					float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
								   * m_PMCOption.fWeight;
					if (m_PMCOption.bUseSumOfLoss)
					{
						a_loss += t_loss;
						b_loss += t_loss;
					}
					else
					{
						if (t_loss > a_loss)
							a_loss = t_loss;
					}
					__ASSERT(a_loss >= 0.0f && b_loss >= 0.0f, "loss > 0이어야 한다.");
				}
				continue;
			}
		}
		else if (tri[2] == pt_a)
		{
			if (tri[0] != pt_b && tri[1] != pt_b)
			{
				combine_modified(a_loss, tri, 2, pt_b, m_PMCOption.bUseSumOfLoss);
				__ASSERT(a_loss >= 0.0f, "loss > 0이어야 한다.");
			}
			else
			{
				// tri collapses.
				if (m_PMCOption.bTriangleWeight)
				{
					float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
								   * m_PMCOption.fWeight;
					if (m_PMCOption.bUseSumOfLoss)
					{
						a_loss += t_loss;
						b_loss += t_loss;
					}
					else
					{
						if (t_loss > a_loss)
							a_loss = t_loss;
					}
					__ASSERT(a_loss >= 0.0f && b_loss >= 0.0f, "loss > 0이어야 한다.");
				}
				continue;
			}
		}
		else
		{
			// no points are a's
			if (tri[0] == pt_b)
			{
				combine_modified(b_loss, tri, 0, pt_a, m_PMCOption.bUseSumOfLoss);
				__ASSERT(b_loss >= 0.0f, "loss > 0이어야 한다.");
			}
			else if (tri[1] == pt_b)
			{
				combine_modified(b_loss, tri, 1, pt_a, m_PMCOption.bUseSumOfLoss);
				__ASSERT(b_loss >= 0.0f, "loss > 0이어야 한다.");
			}
			else if (tri[2] == pt_b)
			{
				combine_modified(b_loss, tri, 2, pt_a, m_PMCOption.bUseSumOfLoss);
				__ASSERT(b_loss >= 0.0f, "loss > 0이어야 한다.");
			}
		}
	}
	__ASSERT(a_loss >= 0.0f && b_loss >= 0.0f, "loss > 0이어야 한다.");

#ifdef _SAME_VERTEXPOS
	float temp_a_loss = GetLossOfSamePosVertex(pt_b, pt_a);
	float temp_b_loss = GetLossOfSamePosVertex(pt_a, pt_b);
	__ASSERT(temp_a_loss >= 0.0f && temp_b_loss >= 0.0f, "loss > 0이어야 한다.");
	if (m_PMCOption.bUseSumOfLoss)
	{
		a_loss += temp_a_loss;
		b_loss += temp_b_loss;
	}
	else
	{
		if (temp_a_loss > a_loss)
			a_loss = temp_a_loss;
		if (temp_b_loss > b_loss)
			b_loss = temp_b_loss;
	}
#endif // _SAME_VERTEXPOS

	// NOTE: What you should do here is check for edges of the object, where
	// the continuity of the mesh is broken. This example code will currently
	// actually collapse these preferentially! Bad idea.

	// We want to lose the point that costs the least.

	// Make sure it's point B that is the least cost by swapping if necessary.
	__ASSERT(a_loss >= 0.0f && b_loss >= 0.0f, "loss > 0이어야 한다.");
	if (b_loss > a_loss)
	{
		swap(pt_a, pt_b);
		b_loss = a_loss;
	}

	// And therefore, the cost of this edge is the cost of losing point B.
	float val = b_loss;

	//__ASSERT(m_PMCOption.bUseSumOfLoss == false || val>0.0f, "Sum of loss value is less than 0");

	// Is it a better candidate than what we've had so far?
	if (val < be_val)
	{
		be_to   = pt_a;
		be_from = pt_b;
		be_val  = val;
		IsOver  = false;
	}
}

// Find a collapse to do, and do it. Returns false if there are no more collapses to do.
bool CN3PMeshCreate::FindACollapse(float& val_so_far)
{
	// First find the best edge to collapse in any material.
	uint16_t* tri;

	float be_val = 10000000000000000000000000000000000.f; // start with a big number
	bool IsOver  = true;

	// The two ends of the edge.
	uint16_t be_index_a;
	uint16_t be_index_b;

	for (tri = m_pIndices; tri < m_pIndices + m_iNumIndices; tri += 3)
	{
		// Try each of the three edges of this triangle.
		TryEdge(tri[0], tri[1], be_val, be_index_a, be_index_b, IsOver);
		TryEdge(tri[1], tri[2], be_val, be_index_a, be_index_b, IsOver);
		TryEdge(tri[2], tri[0], be_val, be_index_a, be_index_b, IsOver);
	}

	// Was there a best edge? If not, collapsing is over.
	if (IsOver)
		return false;

	// The value (cost) of the mesh is the value so far plus the cost of the new edge.
	val_so_far += be_val;

	// Do the collapse
	Collapse(be_index_a, be_index_b, val_so_far);

#ifdef _SAME_VERTEXPOS
	// 같은 점 찾기
	int i;
	for (i = 0; i < m_iNumVertices; ++i)
	{
		if (m_pVertices[be_index_b].x == m_pVertices[i].x
			&& m_pVertices[be_index_b].y == m_pVertices[i].y
			&& m_pVertices[be_index_b].z == m_pVertices[i].z)
		{
			uint16_t index_from = i;
			if (be_index_a < m_iNumVertices && index_from != be_index_a)
				Collapse(be_index_a, index_from, val_so_far);
		}
	}
#endif // _SAME_VERTEXPOS

	return true;
}

void CN3PMeshCreate::CreateCollapseList()
{
	m_iOriginalNumVertices = m_iNumVertices;
	m_iOriginalNumIndices  = m_iNumIndices;

	// make sure it hasn't been done before
	__ASSERT(!m_pCollapses, "Collapses pointer is not NULL!");

	m_pCollapses = new __PMCEdgeCollapse[MAX_COLLAPSES];
	__ASSERT(m_pCollapses,
		"Failed to create collapses buffer"); // note you should really have code to handle failure of malloc
	m_pCollapseUpTo    = m_pCollapses;

											  //	__PMCEdgeCollapse *CollapseUpTo = m_pCollapses;

	m_pAllIndexChanges = new int[MAX_INDEX_CHANGES];
	__ASSERT(m_pAllIndexChanges, "Failed to create Index Change buffer");
	m_iTotalIndexChanges = 0;

	// Keep collapsing until none left to do.
	float start_val      = 0.f;
	while (FindACollapse(start_val))
	{
	}
}

// Converts the mesh into one that can be used by the renderer
CN3PMesh* CN3PMeshCreate::CreateRendererMesh()
{
	CN3PMesh* pPMesh             = new CN3PMesh;

	pPMesh->m_iTotalIndexChanges = m_iTotalIndexChanges;
	if (m_iTotalIndexChanges > 0)
	{
		pPMesh->m_pAllIndexChanges = new int[m_iTotalIndexChanges];
		__ASSERT(pPMesh->m_pAllIndexChanges,
			"Index change buffer is NULL"); // again, you should check for malloc failure
		memcpy(pPMesh->m_pAllIndexChanges, m_pAllIndexChanges,
			m_iTotalIndexChanges * sizeof(m_pAllIndexChanges[0]));
	}

	if (m_pCollapseUpTo - m_pCollapses > 0)
	{
		pPMesh->m_pCollapses = new CN3PMesh::__EdgeCollapse[m_pCollapseUpTo - m_pCollapses];
		__ASSERT(pPMesh->m_pCollapses, "Collpases pointer is NULL!");
	}

	pPMesh->m_iNumCollapses = static_cast<int>(m_pCollapseUpTo - m_pCollapses);

	// Copy the collapses in reverse order, so that the lowest detail collapses are first
	int i;

	float fTempValue = 0.0f;
	for (i = 0; i < pPMesh->m_iNumCollapses; i++)
	{
		__PMCEdgeCollapse& src         = m_pCollapses[pPMesh->m_iNumCollapses - i - 1];
		CN3PMesh::__EdgeCollapse& dest = pPMesh->m_pCollapses[i];

		dest.CollapseTo                = src.CollapseTo;
		dest.iIndexChanges             = static_cast<int>(src.IndexChanges - m_pAllIndexChanges);
		dest.NumIndicesToChange        = src.NumIndicesToChange;
		dest.NumIndicesToLose          = src.NumTrianglesToLose * 3;
		dest.NumVerticesToLose         = src.NumVerticesToLose;
		dest.bShouldCollapse           = (fTempValue == src.Value ? true : false);
		fTempValue                     = src.Value;
	}

	// mesh 정보
	pPMesh->m_iMaxNumIndices  = m_iOriginalNumIndices;
	pPMesh->m_iMaxNumVertices = m_iOriginalNumVertices;
	pPMesh->m_iMinNumIndices  = m_iNumIndices;
	pPMesh->m_iMinNumVertices = m_iNumVertices;

	pPMesh->Create(pPMesh->m_iMaxNumVertices, pPMesh->m_iMaxNumIndices);
	//	pPMesh->m_pIndices = new uint16_t[pPMesh->m_iMaxNumIndices];
	//	__ASSERT(pPMesh->m_pIndices);
	//	pPMesh->m_pVertices = new __VertexT1[pPMesh->m_iMaxNumVertices];
	//	__ASSERT(pPMesh->m_pVertices);

	// The indices can be a straight copy.
	memcpy(pPMesh->m_pIndices, m_pIndices, pPMesh->m_iMaxNumIndices * sizeof(uint16_t));

	// The vertices however must be copied by hand, just in case there's extra data in the
	// PMCVertices (if you have a more complex edge choice function
	for (int j = 0; j < pPMesh->m_iMaxNumVertices; j++)
	{
		pPMesh->m_pVertices[j] = m_pVertices[j];
	}

	return pPMesh;
}

int CN3PMeshCreate::ReGenerate(CN3PMesh* pPMesh)
{
	if (nullptr == pPMesh)
		return -1;

	this->Release();
	this->ConvertFromN3PMesh(pPMesh); // Mesh 로 부터 만들기..
	this->CreateCollapseList();       // Progressive Mesh 처리..

	pPMesh->m_iTotalIndexChanges = m_iTotalIndexChanges;
	if (m_iTotalIndexChanges > 0)
	{
		delete[] pPMesh->m_pAllIndexChanges;
		pPMesh->m_pAllIndexChanges = new int[m_iTotalIndexChanges];
		__ASSERT(pPMesh->m_pAllIndexChanges,
			"Index change buffer is NULL"); // again, you should check for malloc failure
		memcpy(pPMesh->m_pAllIndexChanges, m_pAllIndexChanges,
			m_iTotalIndexChanges * sizeof(m_pAllIndexChanges[0]));
	}

	if (m_pCollapseUpTo - m_pCollapses > 0)
	{
		delete[] pPMesh->m_pCollapses;
		pPMesh->m_pCollapses = new CN3PMesh::__EdgeCollapse[m_pCollapseUpTo - m_pCollapses];
		__ASSERT(pPMesh->m_pCollapses, "Collpases pointer is NULL!");
	}

	pPMesh->m_iNumCollapses = static_cast<int>(m_pCollapseUpTo - m_pCollapses);

	// Copy the collapses in reverse order, so that the lowest detail collapses are first
	int i;

	float fTempValue = 0.0f;
	for (i = 0; i < pPMesh->m_iNumCollapses; i++)
	{
		__PMCEdgeCollapse& src         = m_pCollapses[pPMesh->m_iNumCollapses - i - 1];
		CN3PMesh::__EdgeCollapse& dest = pPMesh->m_pCollapses[i];

		dest.CollapseTo                = src.CollapseTo;
		if (src.IndexChanges != nullptr && m_pAllIndexChanges != nullptr)
			dest.iIndexChanges = static_cast<int>(src.IndexChanges - m_pAllIndexChanges);
		else
			dest.iIndexChanges = 0;
		dest.NumIndicesToChange = src.NumIndicesToChange;
		dest.NumIndicesToLose   = src.NumTrianglesToLose * 3;
		dest.NumVerticesToLose  = src.NumVerticesToLose;
		dest.bShouldCollapse    = (fTempValue == src.Value ? true : false);
		fTempValue              = src.Value;
	}

	// mesh 정보
	pPMesh->m_iMinNumIndices  = m_iNumIndices;
	pPMesh->m_iMinNumVertices = m_iNumVertices;

	pPMesh->Create(m_iOriginalNumVertices, m_iOriginalNumIndices);

	// The indices can be a straight copy.
	memcpy(pPMesh->m_pIndices, m_pIndices, pPMesh->m_iMaxNumIndices * sizeof(uint16_t));

	// The vertices however must be copied by hand, just in case there's extra data in the
	// PMCVertices (if you have a more complex edge choice function
	for (int j = 0; j < pPMesh->m_iMaxNumVertices; j++)
	{
		pPMesh->m_pVertices[j] = m_pVertices[j];
	}

	return 0;
}
// swap "swapper" to the end of the material, updating all references to the vertices being swapped
// swapper번째 버텍스를 버텍스버퍼의 m_iNumVertices-1 번째로 보내고
// ( m_iNumVertices는 collapse 리스트를 만들때마다 하나씩 감소) m_iNumVertices-1번째는 swapper번째로 옮긴다.
// 인덱스 버퍼, collapse리스트 안의 참조 인덱스, pt_to, pt_end도 새로운 인덱스에 맞게 swap 하는 함수
void CN3PMeshCreate::SwapToEnd(uint16_t swapper, __PMCEdgeCollapse* collapses,
	__PMCEdgeCollapse* collapses_end, uint16_t& pt_to, uint16_t& pt_from)
{
	// NOTE: Here you may want to call back into your animation system (for example), so that it knows that
	// the vertex list is being reordered.

	// callback(this, swapper, NumVertices - 1); - tell someone that the vertices are being swapped.

	// pointer to the vertex
	__VertexT1* v = &m_pVertices[swapper];

	// actually swap the vertex data
	swap(*v, m_pVertices[m_iNumVertices - 1]);

	// Then we have to find all references to the swapped vertices, and swap them too.

	// There's references in the triangles still uncollapsed, and the collapsed ones. So run through all
	// the triangles
	int tri_index;
	for (tri_index = 0; tri_index < m_iOriginalNumIndices; tri_index++)
	{
		// Swap any references around
		if (m_pIndices[tri_index] == swapper)
		{
			m_pIndices[tri_index] = m_iNumVertices - 1;
		}
		else if (m_pIndices[tri_index] == m_iNumVertices - 1)
		{
			m_pIndices[tri_index] = swapper;
		}
	}

	// There's references in all the edge collapses for this material.
	for (__PMCEdgeCollapse* c = collapses; c < collapses_end; c++)
	{
		if (c->CollapseTo == swapper)
		{
			c->CollapseTo = m_iNumVertices - 1;
		}
		else if (c->CollapseTo == m_iNumVertices - 1)
		{
			c->CollapseTo = swapper;
		}
	}

	// and the points on the collapsing edge themselves may have been moved
	if (pt_to == m_iNumVertices - 1)
	{
		pt_to = swapper;
	}
	else if (pt_to == swapper)
	{
		pt_to = m_iNumVertices - 1;
	}

	if (pt_from == m_iNumVertices - 1)
	{
		pt_from = swapper;
	}
	else if (pt_from == swapper)
	{
		pt_from = m_iNumVertices - 1;
	}
}

bool CN3PMeshCreate::ConvertFromN3Mesh(CN3Mesh* pN3Mesh) // N3Mesh -> CN3PMeshCreate 로 컨버팅..
{
	if (pN3Mesh == nullptr)
		return false;
	Release();

	// get vertices count , indices count
	m_iNumVertices = pN3Mesh->VertexCount();
	m_iNumIndices  = pN3Mesh->IndexCount();
	if (m_iNumVertices <= 0 || m_iNumIndices <= 0)
		return false;

	// copy vertices
	if (pN3Mesh->Vertices())
	{
		m_pVertices = new __VertexT1[m_iNumVertices];
		CopyMemory(m_pVertices, pN3Mesh->Vertices(), m_iNumVertices * sizeof(__VertexT1));
	}
	else
		return false;

	// copy indices
	if (pN3Mesh->Indices())
	{
		m_pIndices = new uint16_t[m_iNumIndices];
		CopyMemory(m_pIndices, pN3Mesh->Indices(), sizeof(uint16_t) * m_iNumIndices);
	}
	else
		return false;

	m_iOriginalNumVertices = m_iNumVertices;
	m_iOriginalNumIndices  = m_iNumIndices;

	return true;
}

bool CN3PMeshCreate::ConvertFromN3PMesh(CN3PMesh* pN3PMesh)
{
	if (pN3PMesh == nullptr)
		return false;
	Release();

	CN3PMesh* pPMeshTmp = CN3Base::s_MngPMesh.Get(
		pN3PMesh->FileName()); // 이래야 참조 카운트가 하나 늘어서 포인터가 안없어진다.
	CN3PMeshInstance PMeshInst(pN3PMesh);
	PMeshInst.SetLODByNumVertices(pN3PMesh->GetMaxNumVertices());

	// get vertices count , indices count
	m_iNumVertices = PMeshInst.GetNumVertices();
	m_iNumIndices  = PMeshInst.GetNumIndices();
	if (m_iNumVertices <= 0 || m_iNumIndices <= 0)
		return false;

	// copy vertices
	if (PMeshInst.GetVertices())
	{
		m_pVertices = new __VertexT1[m_iNumVertices];
		CopyMemory(m_pVertices, PMeshInst.GetVertices(), m_iNumVertices * sizeof(__VertexT1));
	}
	else
		return false;

	// copy indices
	if (PMeshInst.GetIndices())
	{
		m_pIndices = new uint16_t[m_iNumIndices];
		CopyMemory(m_pIndices, PMeshInst.GetIndices(), sizeof(uint16_t) * m_iNumIndices);
	}
	else
		return false;

	m_iOriginalNumVertices = m_iNumVertices;
	m_iOriginalNumIndices  = m_iNumIndices;

	return true;
}

#ifdef _SAME_VERTEXPOS
float CN3PMeshCreate::GetLossOfSamePosVertex(uint16_t pt_to, uint16_t pt_from)
{
	__ASSERT(pt_to < m_iNumVertices && pt_from < m_iNumVertices && m_pVertices && m_pIndices,
		"Pointer is NULL");
	float fLoss = 0.0f;

	float x     = m_pVertices[pt_from].x;
	float y     = m_pVertices[pt_from].y;
	float z     = m_pVertices[pt_from].z;

	int i;
	for (i = 0; i < m_iNumVertices; ++i)
	{
		// from 과 같은 위치의 Vertex찾기
		if (i != pt_to && i != pt_from && m_pVertices[i].x == x && m_pVertices[i].y == y
			&& m_pVertices[i].z == z)
		{
			// i는 같은 위치를 가진 버텍스의 인덱스
			uint16_t* tri;
			for (tri = m_pIndices; tri < m_pIndices + m_iNumIndices; tri += 3)
			{
				if (tri[0] == i)
				{
					if (tri[1] != pt_to && tri[2] != pt_to)
						combine_modified(fLoss, tri, 0, pt_to, m_PMCOption.bUseSumOfLoss);
					else
						// tri collapses.
						if (m_PMCOption.bTriangleWeight)
						{
							float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
										   * m_PMCOption.fWeight;
							if (m_PMCOption.bUseSumOfLoss)
								fLoss += t_loss;
							else if (t_loss > fLoss)
								fLoss = t_loss;
						}
				}
				else if (tri[1] == i)
				{
					if (tri[0] != pt_to && tri[2] != pt_to)
						combine_modified(fLoss, tri, 1, pt_to, m_PMCOption.bUseSumOfLoss);
					else
						// tri collapses.
						if (m_PMCOption.bTriangleWeight)
						{
							float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
										   * m_PMCOption.fWeight;
							if (m_PMCOption.bUseSumOfLoss)
								fLoss += t_loss;
							else if (t_loss > fLoss)
								fLoss = t_loss;
						}
				}
				else if (tri[2] == i)
				{
					if (tri[0] != pt_to && tri[1] != pt_to)
						combine_modified(fLoss, tri, 2, pt_to, m_PMCOption.bUseSumOfLoss);
					else
						// tri collapses.
						if (m_PMCOption.bTriangleWeight)
						{
							float t_loss = GetTriCollapsesLoss(tri, m_PMCOption.bArea)
										   * m_PMCOption.fWeight;
							if (m_PMCOption.bUseSumOfLoss)
								fLoss += t_loss;
							else if (t_loss > fLoss)
								fLoss = t_loss;
						}
				}
			}
		}
	}
	return fLoss;
}
#endif // _SAME_VERTEXPOS
