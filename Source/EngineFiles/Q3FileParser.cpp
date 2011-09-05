#include "Q3FileParser.h"
#include <set>
#include "ResourceCache\ResCache2.h"
#include "EngineFiles\Game.h"

DWORD Vertex::FVF = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2);

void MapFileParser::Init(std::string fileName)
{
	file.open( fileName.c_str(), std::ios::in | std::ios::binary);
	if (!file.good())
		file.close();
}

void MapFileParser::ReadString(char* c, int length)
{
	if (c)
	{
		char t;
		for (int i = 0; i < length; i++)
		{
			file.get(t);
			c[i] = t;
		}
	}
}

float MapFileParser::ReadFloat()
{
	float t;
	file.read((char *)&t, sizeof(float));
	return t;
}

int MapFileParser::ReadInt()
{
	int t;
	file.read((char *)&t, sizeof(int));
	return t;
}

DWORD MapFileParser::ReadDWORD()
{
	DWORD t;
	file.read((char *)&t, sizeof(DWORD));
	return t;
}

unsigned char MapFileParser::ReadByte()
{
	unsigned char t;
	file.read((char *)&t, sizeof(unsigned char));
	return t;
}

shared_ptr<Q3Map> MapFileParser::ReadMap()
{
	shared_ptr<Q3Map> q (new Q3Map());
	if(file.good())
	{
		Header h;
		h = ReadHeaderLump();
		ReadEntityLump(h.direntries[0]);
		q->textList = ReadTextureLump(h.direntries[1]);
		q->planeList = ReadPlaneLump(h.direntries[2]);
		q->nodeList = ReadNodeLump(h.direntries[3]);
		q->leafList = ReadLeafLump(h.direntries[4]);
		q->leafFaceList = ReadLeafFaceLump(h.direntries[5]);
		q->leafBrushList = ReadLeafBrushLump(h.direntries[6]);
		//ReadModelLump(h.direntries[7]);
		q->brushList = ReadBrushLump(h.direntries[8]);
		q->brushSideList = ReadBrushSideLump(h.direntries[9]);
		q->vertList = ReadVertexLump(h.direntries[10]);
		q->meshVertList = ReadMeshVertLump(h.direntries[11]);
		//ReadEffectsLump(h.direntries[12]);
		q->faceList = ReadFaceLump(h.direntries[13]);
		//ReadLightMapLump(h.direntries[14]);
		//ReadLightVolLump(h.direntries[15]);
		q->visData = ReadVisdata(h.direntries[16]);

	}
	return q;
}

Header MapFileParser::ReadHeaderLump()
{
	Header head;
	ReadString(head.magic, 4);
	head.version = ReadInt();
	for (int i = 0; i < 17; i++)
		head.direntries[i] = ReadDirEntry();

	return head;
}

Direntry MapFileParser::ReadDirEntry()
{
	Direntry ent;
	file.read((char *) &ent, sizeof(ent));

	return ent;
}

Entity MapFileParser::ReadEntityLump(Direntry dir)
{
	Entity ent;
	ent.ents = "";
	char t;
	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < dir.length; i++)
	{
		file.get(t);
		ent.ents += t;
	}
	
	return ent;
}

TextureList MapFileParser::ReadTextureLump(Direntry dir)
{
	TextureList l;
	int numOfTextures = dir.length / sizeof(Texture);
	Texture	t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfTextures; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

PlaneList MapFileParser::ReadPlaneLump(Direntry dir)
{
	PlaneList l;
	int numOfPlanes = dir.length / sizeof(Plane);
	Planeq t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfPlanes; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

NodeList MapFileParser::ReadNodeLump(Direntry dir)
{
	NodeList l;
	int numOfNode = dir.length / sizeof(Node);
	Node t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfNode; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

LeafList MapFileParser::ReadLeafLump(Direntry dir)
{
	LeafList l;
	int numOfLeaves = dir.length / sizeof(Leaf);
	Leaf t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfLeaves; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

LeafFaceList MapFileParser::ReadLeafFaceLump(Direntry dir)
{
	LeafFaceList l;
	int numOfLeaves = dir.length / sizeof(Leafface);
	Leafface t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfLeaves; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

LeafBrushList MapFileParser::ReadLeafBrushLump(Direntry dir)
{
	LeafBrushList l;
	int numOfLeaves = dir.length / sizeof(Leafbrush);
	Leafbrush t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfLeaves; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

ModelList MapFileParser::ReadModelLump(Direntry dir)
{
	ModelList l;
	int numOfModels = dir.length / sizeof(Model);
	Model t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfModels; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

BrushList MapFileParser::ReadBrushLump(Direntry dir)
{
	BrushList l;
	int numOfBrushes = dir.length / sizeof(Brush);
	Brush t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfBrushes; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

BrushSideList MapFileParser::ReadBrushSideLump(Direntry dir)
{
	BrushSideList l;
	int numOfBrushes = dir.length / sizeof(Brushside);
	Brushside t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfBrushes; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

VertexList MapFileParser::ReadVertexLump(Direntry dir)
{
	VertexList l;
	int numOfVertex = dir.length / sizeof(Vertex);
	Vertex t;

	int sizeofLong = sizeof(unsigned long);

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfVertex; i++)
	{
		t.position[0] = ReadFloat();
		t.position[1] = ReadFloat();
		t.position[2] = ReadFloat();
		t.texcoord[0] = ReadFloat();
		t.texcoord[1] = ReadFloat();
		t.lightmapcoord[0] = ReadFloat();
		t.lightmapcoord[1] = ReadFloat();

		t.normal[0] = ReadFloat();
		t.normal[1] = ReadFloat();
		t.normal[2] = ReadFloat();

		t.color = ReadDWORD();
		t.color = g_White;

		l.push_back(t);
	}

	return l;
}

MeshVertsList MapFileParser::ReadMeshVertLump(Direntry dir)
{
	MeshVertsList l;
	int numOfVertex = dir.length / sizeof(Meshverts);
	Meshverts t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfVertex; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

EffectsList MapFileParser::ReadEffectsLump(Direntry dir)
{
	EffectsList l;
	int numOfEffects = dir.length / sizeof(Effects);
	Effects t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfEffects; i++)
	{
		file.read((char *) &t, sizeof(t));
		l.push_back(t);
	}

	return l;
}

FaceList MapFileParser::ReadFaceLump(Direntry dir)
{
	FaceList l;
	int numOfFaces = dir.length / sizeof(Face);
	Face t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfFaces; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

LightmapList MapFileParser::ReadLightMapLump(Direntry dir)
{
	LightmapList l;
	int numOfFaces = dir.length / sizeof(Lightmap);
	Lightmap t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfFaces; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

LightVolList MapFileParser::ReadLightVolLump(Direntry dir)
{
	LightVolList l;
	int numOfFaces = dir.length / sizeof(Lightvol);
	Lightvol t;

	file.seekg(dir.offset, std::ios::beg);
	for (int i = 0; i < numOfFaces; i++)
	{
		file.read((char *) &t, sizeof(t));

		l.push_back(t);
	}

	return l;
}

Visdata MapFileParser::ReadVisdata(Direntry dir)
{
	Visdata t;

	file.seekg(dir.offset, std::ios::beg);
	t.n_vecs = ReadInt();
	t.sz_vecs = ReadInt();

	for (int i = 0; i < t.n_vecs * t.sz_vecs; i++)
	{
		t.vecs.push_back(ReadByte());
	}

	return t;
}



int Q3Map::FindLeaf(Vec3 v)
{
	int index = 0;

	while (index >=0)
	{
		Node n = nodeList[index];
		Planeq p = planeList[n.plane];

		float distance = p.normal[0] * v.x + p.normal[1] * v.y + p.normal[2] * v.z;
		distance -= p.dist;

		if (distance >= 0)
			index = n.children[0];
		else
			index = n.children[1];
	}

	return -index - 1;
}

bool Q3Map::IsClusterVisible(int visCluster, int testCluster)
{
	if (visData.vecs.empty() || (visCluster <0))
		return true;

	int i = (visCluster * visData.sz_vecs) + (testCluster >> 3);
	unsigned int visSet = visData.vecs[i];
	int j = testCluster & 7;

	return (visSet & (1 << (j))) != 0;
}

bool Q3Map::LeafVisibility(shared_ptr<CameraNode> camera, Leaf leaf)
{
	Vec3 minV(leaf.mins[0], leaf.mins[1], leaf.mins[2]);
	Vec3 maxV(leaf.maxs[0], leaf.maxs[1], leaf.maxs[2]);
	
	Mat4x4 toWorld, fromWorld;
	camera->VGet()->Transform(&toWorld, &fromWorld);

	minV = fromWorld.Xform(minV);
	maxV = fromWorld.Xform(maxV);

	return camera->GetFrustum().Inside(minV, maxV);
}

HRESULT Q3Map::VPreRender(Scene *pScene)
{
	
	SceneNode::VPreRender(pScene);

	std::set<int> alreadyVisible;
	std::set<int>::iterator avIt;
	visibleFaces.clear();

	LeafList::iterator it;

	shared_ptr<CameraNode> camera = pScene->GetCamera();
	Vec3 v = camera->VGet()->ToWorld().GetPosition();
	int cameraLeaf = FindLeaf(v);
	int visCluster = leafList[cameraLeaf].cluster;

	for (it = leafList.begin(); it < leafList.end(); it++)
	{
		if (IsClusterVisible(visCluster, (*it).cluster))
			if(LeafVisibility(camera, (*it)))
			{
				for (int i = 0; i < (*it).n_leaffaces; i++)
				{
					int face = leafFaceList[i + (*it).leafface].face;
					avIt = alreadyVisible.find(face);
					if (avIt == alreadyVisible.end())
					{
						alreadyVisible.insert(face);
						visibleFaces.push_back(face);
					}
				}
			}
	}

	return S_OK;
}


HRESULT Q3Map::VRender(Scene *pScene)
{
	
	std::vector<int>::iterator it;

	//DXUTGetD3DDevice()->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );

	/*DXUTGetD3DDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DXUTGetD3DDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	DXUTGetD3DDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );*/


	DXUTGetD3DDevice()->SetIndices(m_pIndices);
	DXUTGetD3DDevice()->SetStreamSource(0, m_pVerts, 0, sizeof(Vertex));
	DXUTGetD3DDevice()->SetFVF( Vertex::FVF );

	for (it = visibleFaces.begin(); it < visibleFaces.end(); it++)
	{
		if (faceList[(*it)].type == 1 || faceList[(*it)].type == 3) 
		{
			int vert = faceList[(*it)].vertex;
			int lastVert = faceList[(*it)].n_vertex;
			int meshVert = faceList[(*it)].meshvert;
			int numPrim = faceList[(*it)].n_meshverts/3;
			int text = faceList[(*it)].texture;

			Texture t = textList[text];

			if (t.flags == 1044)
			{
					HRESULT t = DXUTGetD3DDevice()->SetTexture( 0, m_pTexture2);
			}
			else
			{
					HRESULT t = DXUTGetD3DDevice()->SetTexture( 0, m_pTexture);
			}


			HRESULT test = DXUTGetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vert, 0, lastVert, meshVert, numPrim );
		}
	}

	DXUTGetD3DDevice()->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	DXUTGetD3DDevice()->SetTexture(0, NULL);

	return S_OK;
}

HRESULT Q3Map::VOnRestore(Scene *pScene)
{
	
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVerts);
	SAFE_RELEASE(m_pIndices);

	SceneNode::VOnRestore(pScene);

	Resource resource("background2.bmp");
	int size = g_App->m_ResCache->Create(resource);

	assert(size);
	char *textureBuffer = (char *)g_App->m_ResCache->Get(resource);

	if (FAILED( D3DXCreateTextureFromFileInMemory( DXUTGetD3DDevice(),
				textureBuffer, resource.m_size, &m_pTexture) ) )
		return E_FAIL;

	Resource resource2("black.bmp");
	size = g_App->m_ResCache->Create(resource);

	assert(size);
	textureBuffer = (char *)g_App->m_ResCache->Get(resource2);

	if (FAILED( D3DXCreateTextureFromFileInMemory( DXUTGetD3DDevice(),
				textureBuffer, resource.m_size, &m_pTexture2) ) )
		return E_FAIL;

	/*D3DVERTEXELEMENT9 vertexElements[] = {
			{0, 0,  D3DDECLTYPE_FLOAT3,     D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT,  0},
			{0, 12, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,   0},
			{0, 20, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,   1},
			{0, 28, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0},
			{0, 40, D3DDECLTYPE_D3DCOLOR,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,      0},
			D3DDECL_END()
		};
	
	//SetRadius( sqrt(2.0f * m_params->m_Squares) );

	vertexDecleration = 0;
	if ( FAILED (DXUTGetD3DDevice()->CreateVertexDeclaration(vertexElements, &vertexDecleration)))
		return E_FAIL;
	DXUTGetD3DDevice()->SetVertexDeclaration(vertexDecleration);*/


	int verts = (int)vertList.size();
	if ( FAILED( DXUTGetD3DDevice()->CreateVertexBuffer(verts*sizeof(Vertex),
		D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_DEFAULT, &m_pVerts, NULL) ) )
		return E_FAIL;
		
	Vertex * pVertices ;
	if ( FAILED (m_pVerts->Lock(0, 0, (void**)&pVertices, 0 )))
		return E_FAIL;

	// need to copy all the vertices into the vertex buffer from vertexList.
	VertexList::iterator it;
	int i = 0;
	for (it = vertList.begin(); it < vertList.end(); it++, i++)
	{
		Vertex* v = &pVertices[i];

		v->position[0] = (*it).position[0];
		v->position[1] = (*it).position[1];
		v->position[2] = (*it).position[2];

		v->lightmapcoord[0] = (*it).lightmapcoord[0];
		v->lightmapcoord[1] = (*it).lightmapcoord[1];
	
		v->texcoord[0] =  (*it).texcoord[0];
		v->texcoord[1] =  (*it).texcoord[1];

		v->color =  (*it).color;

		v->normal[0] = (*it).normal[0];
		v->normal[1] = (*it).normal[1];
		v->normal[2] = (*it).normal[2];
	}

	m_pVerts->Unlock();

	int meshVertCount = (int)meshVertList.size();
	if ( FAILED( DXUTGetD3DDevice()->CreateIndexBuffer(sizeof(WORD) * meshVertCount,
				D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndices, NULL) ) )
		return E_FAIL;
	
	WORD *pIndices;
	if ( FAILED( m_pIndices->Lock(0, 0, (void**) &pIndices, 0 ) ) )
		return E_FAIL;
	
	MeshVertsList::iterator mit;
	int j = 0;
	for (mit = meshVertList.begin(); mit < meshVertList.end(); mit++, j++)
	{
		*(pIndices + j) = (WORD)(*mit).offset;
	}

	m_pIndices->Unlock();
	
	
	return S_OK;
}

Q3Map::~Q3Map()
{
	SAFE_RELEASE(m_pTexture2);
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVerts);
	SAFE_RELEASE(m_pIndices);
}

TraceOut Q3Map::Trace(Vec3 inStart, Vec3 inEnd)
{
	return Trace(inStart, inEnd, 0, 0);
}

TraceOut Q3Map::Trace(Vec3 inStart, Vec3 inEnd, int type, float size)
{
	TraceOut t;
	t.outputAllSolid = false;
	t.outputStartsOut = true;
	t.outputFraction = 1.0f;

	float offset;

	if (type == 0) // ray
	{
		offset = 0;
	}
	else if (type == 1)
	{
		offset = size;
	}

	CheckNode(0, 0.0f, 1.0f, inStart, inEnd, type, offset, &t);

	if (t.outputFraction == 1.0f)
	{
		t.outputEnd = inEnd;
	}
	else
	{
		t.outputEnd = inStart + t.outputFraction * (inEnd - inStart);
	}
	return t;
}

void Q3Map::CheckNode(int nodeIndex, float startFraction, float endFraction, Vec3 start, Vec3 end, int type, float offset, TraceOut* output)
{
	if (nodeIndex < 0)
	{
		Leaf leaf = leafList[-(nodeIndex + 1)];
		for (int i = 0; i < leaf.n_leafbrush; i++)
		{
			Brush brush = brushList[leafBrushList[leaf.leafbrush + i].brush];
			if (brush.n_brushside > 0 && textList[brush.texture].contents & 1)
			{
				CheckBrush(brush, start, end, offset, output);
			}
		}

		return;
	}
	Node node = nodeList[nodeIndex];
	Planeq plane = planeList[node.plane];
	Vec3 planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);

	float startDistance = start.Dot(planeNormal) - plane.dist;
	float endDistance = end.Dot(planeNormal) - plane.dist;
	

	if (startDistance >= offset && endDistance >= offset)
	{
		// on the front side of the plane
		CheckNode(node.children[0], startFraction, endFraction, start, end, type, offset, output);
	}
	else if (startDistance < -offset && endDistance < -offset)
	{
		// on the back side of the plane
		CheckNode(node.children[1], startFraction, endFraction, start, end, type, offset, output);
	}
	else
	{
		// on both sides of the plane
		int side;
		float frac1, frac2, middleFrac;
		Vec3 middle;

		// split the line to check both sides of the plane
		if (startDistance < endDistance)
		{
			side = 1;
			float inverseDistance = 1.0f / (startDistance - endDistance);
			frac1 = (startDistance - offset + EPSILON) * inverseDistance;
			frac2 = (startDistance + offset + EPSILON) * inverseDistance;
		}
		else if (endDistance < startDistance)
		{
			side = 0;
			float inverseDistance = 1.0f / (startDistance - endDistance);
			frac1 = (startDistance + offset + EPSILON) * inverseDistance;
			frac2 = (startDistance - offset - EPSILON) * inverseDistance;
		}
		else
		{
			side = 0;
			frac1 = 1.0f;
			frac2 = 0.0f;
		}

		if (frac1 < 0.0f) frac1 = 0.0f;
		else if (frac1 > 1.0f) frac1 = 1.0f;

		if (frac2 < 0.0f) frac2 = 0.0f;
		else if (frac2 > 1.0f) frac2 = 1.0f;
		
		// find the middle point for the front side
		middleFrac = startFraction + (endFraction - startFraction) * frac1;

		middle = start + frac1 * (end - start);

		CheckNode( node.children[side], startFraction, middleFrac, start, middle, type, offset, output);

		// find the middle point for the front side
		middleFrac = startFraction + (endFraction - startFraction) * frac2;

		middle = start + frac2 * (end - start);

		CheckNode( node.children[!side], middleFrac, endFraction, middle, end, type, offset, output);
	}
}

void Q3Map::CheckBrush(Brush b, Vec3 inputStart, Vec3 inputEnd, float offset, TraceOut* output)
{
	if (output == NULL)
		return;

	bool startsOut = false;
	bool endsOut = false;
	float startFrac = -1.0f;
	float endFrac = 1.0f;
	Planeq planeOut = output->outputPlane;

	for (int i = 0; i < b.n_brushside; i++)
	{
		Brushside brushSide = brushSideList[b.brushside + i];
		Planeq plane = planeList[brushSide.plane];
		Vec3 planeNormal(plane.normal[0], plane.normal[1], plane.normal[2]);
		
		float startDistance = inputStart.Dot(planeNormal) - (plane.dist + offset);
		float endDistance = inputEnd.Dot(planeNormal) - (plane.dist + offset);

		if (startDistance > 0)
			startsOut = true;
		if (endDistance > 0)
			endsOut = true;

		if (startDistance > 0 && endDistance > 0)
			return;

		if (startDistance <= 0 && endDistance <= 0)
			continue;

		if (startDistance > endDistance)
		{
			float frac = (startDistance - EPSILON) / (startDistance - endDistance);
			if (frac > startFrac)
			{
				startFrac = frac;
				planeOut = plane;
			}
		}
		else
		{
			float frac = (startDistance + EPSILON) / (startDistance - endDistance);
			if (frac < endFrac)
				endFrac = frac;
		}
	}

	if (startsOut == false)
	{
		output->outputStartsOut = false;
		if (endsOut == false)
			output->outputAllSolid = true;
		return;
	}
	
	if (startFrac < endFrac)
	{
		if (startFrac > -1 && startFrac < output->outputFraction)
		{
			if (startFrac < 0)
				startFrac = 0;
			output->outputFraction = startFrac;
			output->outputPlane = planeOut;
		}
	}
}