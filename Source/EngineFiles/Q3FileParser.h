#pragma once

#include "StdHeader.h"
#include "SceneNode.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>


#pragma pack(1)
struct Direntry
{
	int				offset;
	int				length;
};

struct Header
{
	char			magic[4];
	int				version;
	Direntry		direntries[17];
};

struct Entity
{
	std::string			ents;
};

struct Texture
{
	char			name[64];
	int				flags;
	int				contents;
};

struct Planeq
{
	float			normal[3];
	float			dist;
};

struct Node
{
	int				plane;
	int				children[2];
	int				mins[3];
	int				maxs[3];
};

struct Leaf
{
	int				cluster;
	int				area;
	int				mins[3];
	int				maxs[3];
	int				leafface;
	int				n_leaffaces;
	int				leafbrush;
	int				n_leafbrush;
};

struct Leafface
{
	int				face;
};

struct Leafbrush
{
	int				brush;
};

struct Model
{
	float			mins[3];
	float			maxs[3];
	int				face;
	int				n_face;
	int				brush;
	int				n_brush;
};

struct Brush
{
	int				brushside;
	int				n_brushside;
	int				texture;
};

struct Brushside
{
	int				plane;
	int				texture;
};

struct Vertex
{
	float			position[3];
	float			normal[3];
	DWORD			color;
	float			texcoord[2];
	float			lightmapcoord[2];

	static DWORD	FVF;
};

struct Meshverts
{
	int				offset;
};

struct Effects
{
	char			name[64];
	int				brush;
	int				unknown;
};

struct Face
{
	int				texture;
	int				effect;
	int				type;
	int				vertex;
	int				n_vertex;
	int				meshvert;
	int				n_meshverts;
	int				lm_index;
	int				lm_start[2];
	int				lm_size[2];
	float			lm_origins[3];
	float			lm_vecs[2][3];
	float			normal[3];
	int				size[2];
};

struct Lightmap
{
	unsigned int	 map[128][128][3];
};

struct Lightvol
{
	unsigned int	ambient[3];
	unsigned int	directional[3];
	unsigned int	dir[2];
};
#pragma pack()

struct Visdata
{
	int				n_vecs;
	int				sz_vecs;
	std::vector<unsigned int>	vecs;
};

const float EPSILON = 0.03125;


typedef std::vector<Texture> TextureList;
typedef std::vector<Planeq> PlaneList;
typedef std::vector<Node> NodeList;
typedef std::vector<Leaf> LeafList;
typedef std::vector<Leafface> LeafFaceList;
typedef std::vector<Leafbrush> LeafBrushList;
typedef std::vector<Model> ModelList;
typedef std::vector<Brush> BrushList;
typedef std::vector<Brushside> BrushSideList;
typedef std::vector<Vertex> VertexList;
typedef std::vector<Meshverts> MeshVertsList;
typedef std::vector<Effects> EffectsList;
typedef std::vector<Face> FaceList;
typedef std::vector<Lightmap> LightmapList;
typedef std::vector<Lightvol> LightVolList;

struct TraceOut
{
	float	outputFraction;
	Vec3	outputEnd;
	bool	outputStartsOut;
	bool	outputAllSolid;
	Planeq	outputPlane;
};

class Q3Map : public SceneNode
{
	LPDIRECT3DVERTEXBUFFER9			m_pVerts;
	IDirect3DVertexDeclaration9*	vertexDecleration;
	LPDIRECT3DINDEXBUFFER9			m_pIndices;
	LPDIRECT3DTEXTURE9				m_pTexture;
	LPDIRECT3DTEXTURE9				m_pTexture2;


	void CheckNode(int nodeIndex, float startFraction, float endFraction, Vec3 start, Vec3 end, int type, float offset, TraceOut* output);
	void CheckBrush(Brush b, Vec3 inputStart, Vec3 inputEnd, float offset, TraceOut* output);

public:
	TextureList textList;
	PlaneList planeList;
	NodeList nodeList;
	Visdata	visData;
	LeafList leafList;
	FaceList faceList;
	VertexList vertList;
	MeshVertsList meshVertList;
	LeafFaceList leafFaceList;
	BrushList brushList;
	LeafBrushList leafBrushList;
	BrushSideList brushSideList;
	std::vector<int> visibleFaces;

	Q3Map(): SceneNode()
	{
		m_pVerts = NULL;
		m_pIndices = NULL;
		m_pTexture = NULL;
		vertexDecleration = 0;
		m_props.SetHasAlpha(false);
	}
	~Q3Map();
	
	int FindLeaf(Vec3);
	bool IsClusterVisible(int visCluster, int textCluster);

	HRESULT VOnRestore(Scene *pScene);
	HRESULT VPreRender(Scene *pScene);
	HRESULT VRender(Scene *pScene);

	bool VIsVisible(Scene *pScene) {return true;}
	bool LeafVisibility(shared_ptr<CameraNode> camera, Leaf leaf);

	TraceOut Trace(Vec3 inStart, Vec3 inEnd);
	TraceOut Trace(Vec3 inStart, Vec3 inEnd, int type, float size);

};

class MapFileParser
{
public:
	MapFileParser(){}
	void Init(std::string fileName);
	shared_ptr<Q3Map> ReadMap();

private:
	
	int ReadInt();
	void ReadString(char* c, int length);
	float ReadFloat();
	unsigned char ReadByte();
	DWORD ReadDWORD();

	Direntry		ReadDirEntry();
	Header			ReadHeaderLump();
	Entity			ReadEntityLump(Direntry dir);
	TextureList		ReadTextureLump(Direntry dir);
	PlaneList		ReadPlaneLump(Direntry dir);
	NodeList		ReadNodeLump(Direntry dir);
	LeafList		ReadLeafLump(Direntry dir);
	LeafFaceList	ReadLeafFaceLump(Direntry dir);
	LeafBrushList	ReadLeafBrushLump(Direntry dir);
	ModelList		ReadModelLump(Direntry dir);
	BrushList		ReadBrushLump(Direntry dir);
	BrushSideList	ReadBrushSideLump(Direntry dir);
	VertexList		ReadVertexLump(Direntry dir);
	MeshVertsList	ReadMeshVertLump(Direntry dir);
	EffectsList		ReadEffectsLump(Direntry dir);
	FaceList		ReadFaceLump(Direntry dir);
	LightmapList	ReadLightMapLump(Direntry dir);
	LightVolList	ReadLightVolLump(Direntry dir);
	Visdata			ReadVisdata(Direntry dir);
	
	std::ifstream file;
};
