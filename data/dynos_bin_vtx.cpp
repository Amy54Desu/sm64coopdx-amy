#include "dynos.cpp.h"
extern "C" {
#include "engine/graph_node.h"
#include "pc/lua/utils/smlua_gfx_utils.h"
}

#define F32VTX_SENTINEL_0 0x3346
#define F32VTX_SENTINEL_1 0x5632
#define F32VTX_SENTINEL_2 0x5854

static inline bool ShouldUseF32Vtx(DataNode<Vtx>* aNode) {
    for (u32 i = 0; i != aNode->mSize; ++i) {
        for (u32 j = 0; j != 3; ++j) {
            if (aNode->mData[i].n.ob[j] < -0x7FFF ||
                aNode->mData[i].n.ob[j] > +0x7FFF) {
                return true;
            }
        }
    }
    return false;
}

static inline bool IsUsingF32Vtx(Vec3f ob) {
    return ob[0] == F32VTX_SENTINEL_0 &&
           ob[1] == F32VTX_SENTINEL_1 &&
           ob[2] == F32VTX_SENTINEL_2;
}

  /////////////
 // Parsing //
/////////////

DataNode<Vtx>* DynOS_Vtx_Parse(GfxData* aGfxData, DataNode<Vtx>* aNode) {
    if (aNode->mData) return aNode;

    // Vertex data
    aNode->mSize = (u32) (aNode->mTokens.Count() / 10);
    aNode->mData = vtx_allocate_internal(NULL, aNode->mSize);
    for (u32 i = 0; i != aNode->mSize; ++i) {
        f32 px = (f32) aNode->mTokens[10 * i + 0].ParseFloat();
        f32 py = (f32) aNode->mTokens[10 * i + 1].ParseFloat();
        f32 pz = (f32) aNode->mTokens[10 * i + 2].ParseFloat();
        u16 fl = (u16) aNode->mTokens[10 * i + 3].ParseInt();
        s16 tu = (s16) aNode->mTokens[10 * i + 4].ParseInt();
        s16 tv = (s16) aNode->mTokens[10 * i + 5].ParseInt();
        u8 nx = (u8) aNode->mTokens[10 * i + 6].ParseInt();
        u8 ny = (u8) aNode->mTokens[10 * i + 7].ParseInt();
        u8 nz = (u8) aNode->mTokens[10 * i + 8].ParseInt();
        u8 a  = (u8) aNode->mTokens[10 * i + 9].ParseInt();
        aNode->mData[i] = { { { px, py, pz }, fl, { tu, tv }, { nx, ny, nz, a } } };
    }
    aNode->mLoadIndex = aGfxData->mLoadIndex++;
    return aNode;
}

  /////////////
 // Writing //
/////////////

void DynOS_Vtx_Write(BinFile* aFile, GfxData* aGfxData, DataNode<Vtx> *aNode) {
    if (!aNode->mData) return;

    // Header
    aFile->Write<u8>(DATA_TYPE_VERTEX);
    aNode->mName.Write(aFile);

    // Data
    bool shouldUseF32Vtx = ShouldUseF32Vtx(aNode);
    if (shouldUseF32Vtx) {
        aFile->Write<u32>(aNode->mSize + 1);

        // Write sentinel
        aFile->Write<s16>(F32VTX_SENTINEL_0);
        aFile->Write<s16>(F32VTX_SENTINEL_1);
        aFile->Write<s16>(F32VTX_SENTINEL_2);
        aFile->Write<s16>(0);
        aFile->Write<s16>(0);
        aFile->Write<s16>(0);
        aFile->Write<s8> (0);
        aFile->Write<s8> (0);
        aFile->Write<s8> (0);
        aFile->Write<u8> (0);
    } else {
        aFile->Write<u32>(aNode->mSize);
    }
    for (u32 i = 0; i != aNode->mSize; ++i) {
        if (shouldUseF32Vtx) {
            aFile->Write<f32>(aNode->mData[i].n.ob[0]);
            aFile->Write<f32>(aNode->mData[i].n.ob[1]);
            aFile->Write<f32>(aNode->mData[i].n.ob[2]);
        } else {
            aFile->Write<s16>(aNode->mData[i].n.ob[0]);
            aFile->Write<s16>(aNode->mData[i].n.ob[1]);
            aFile->Write<s16>(aNode->mData[i].n.ob[2]);
        }
        aFile->Write<s16>(aNode->mData[i].n.flag);
        aFile->Write<s16>(aNode->mData[i].n.tc[0]);
        aFile->Write<s16>(aNode->mData[i].n.tc[1]);
        aFile->Write<s8> (aNode->mData[i].n.n[0]);
        aFile->Write<s8> (aNode->mData[i].n.n[1]);
        aFile->Write<s8> (aNode->mData[i].n.n[2]);
        aFile->Write<u8> (aNode->mData[i].n.a);
    }
}

  /////////////
 // Reading //
/////////////

void DynOS_Vtx_Load(BinFile *aFile, GfxData *aGfxData) {
    DataNode<Vtx> *_Node = New<DataNode<Vtx>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    bool isUsingF32Vtx = false;
    _Node->mSize = aFile->Read<u32>();
    _Node->mData = vtx_allocate_internal(NULL, _Node->mSize);
    for (u32 i = 0; i != _Node->mSize; ++i) {
        if (isUsingF32Vtx) {
            _Node->mData[i].n.ob[0] = aFile->Read<f32>();
            _Node->mData[i].n.ob[1] = aFile->Read<f32>();
            _Node->mData[i].n.ob[2] = aFile->Read<f32>();
        } else {
            _Node->mData[i].n.ob[0] = aFile->Read<s16>();
            _Node->mData[i].n.ob[1] = aFile->Read<s16>();
            _Node->mData[i].n.ob[2] = aFile->Read<s16>();
        }
        _Node->mData[i].n.flag  = aFile->Read<s16>();
        _Node->mData[i].n.tc[0] = aFile->Read<s16>();
        _Node->mData[i].n.tc[1] = aFile->Read<s16>();
        _Node->mData[i].n.n[0]  = aFile->Read<s8> ();
        _Node->mData[i].n.n[1]  = aFile->Read<s8> ();
        _Node->mData[i].n.n[2]  = aFile->Read<s8> ();
        _Node->mData[i].n.a     = aFile->Read<u8> ();

        // Check sentinel on first vertex
        if (!isUsingF32Vtx && i == 0 && IsUsingF32Vtx(_Node->mData[i].n.ob)) {
            _Node->mSize--; i--;
            isUsingF32Vtx = true;
        }
    }

    // Billboard check
    if (!(_Node->mFlags & GRAPH_EXTRA_FORCE_3D)) {
        Vec3f pn = { 0, 0, 0 };
        for (u32 i = 2; i < _Node->mSize; ++i) {
            Vec3f p0; vec3f_copy(p0, _Node->mData[i - 2].v.ob);
            Vec3f p1; vec3f_copy(p1, _Node->mData[i - 1].v.ob);
            Vec3f p2; vec3f_copy(p2, _Node->mData[i - 0].v.ob);
            Vec3f v0; vec3f_dif(v0, p0, p1);
            Vec3f v1; vec3f_dif(v1, p1, p2);
            Vec3f vn; vec3f_cross(vn, v0, v1);
            if (vn[0] != 0.f || vn[1] != 0.f || vn[2] != 0.f) { // skip zero normals
                vec3f_normalize(vn);
                if (pn[0] != 0.f || pn[1] != 0.f || pn[2] != 0.f) { // don't compare to zero normal
                    f32 dot = vec3f_dot(vn, pn);
                    if (dot < 0.9f) { // Points don't form a plane -> no billboard
                        _Node->mFlags |= GRAPH_EXTRA_FORCE_3D;
                        break;
                    }
                }
                vec3f_copy(pn, vn);
            }
        }
    }

    // Append
    aGfxData->mVertices.Add(_Node);
}
