/*
* 3DS Max Exporter
*
* 작성자: bumpsgoodman
* 작성일: 2023.01.31
*/

#define _CRT_SECURE_NO_WARNINGS

#include "Exporter.h"
#include "Defines.h"

// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
#define UW3DEXP_CLASS_ID	Class_ID(0x4f3d6ef1, 0x39dc4eb0)

HINSTANCE g_hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        MaxSDK::Util::UseLanguagePackLocale();
        g_hInstance = hinstDLL;
        DisableThreadLibraryCalls(g_hInstance);
    }

    return (TRUE);
}

__declspec(dllexport) const TCHAR* LibDescription()
{
    return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec(dllexport) int LibNumberClasses()
{
    return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
    switch (i)
    {
    case 0: return GetUW3dExpDesc();
    default: return 0;
    }
}

__declspec(dllexport) ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec(dllexport) ULONG CanAutoDefer()
{
    return 1;
}

class UW3dExpClassDesc :public ClassDesc {
public:
    int	IsPublic() { return 1; }
    void* Create(BOOL loading = FALSE) { return new Exporter; }
    const TCHAR* ClassName() { return GetString(IDS_UW3DEXP); }
    const TCHAR* NonLocalizedClassName() { return _T("UW3dExporter"); }
    SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
    Class_ID ClassID() { return UW3DEXP_CLASS_ID; }
    const TCHAR* Category() { return GetString(IDS_CATEGORY); }
};

static UW3dExpClassDesc s_uw3dExpDesc;

ClassDesc* GetUW3dExpDesc()
{
    return &s_uw3dExpDesc;
}

Exporter::~Exporter()
{
}

int Exporter::ExtCount()
{
    return 1;
}

const TCHAR* Exporter::Ext(int n)
{
    switch (n)
    {
    case 0:
        return _T("uw3d");
    }

    return _T("");
}

const TCHAR* Exporter::LongDesc()
{
    return GetString(IDS_LONGDESC);
}

const TCHAR* Exporter::ShortDesc()
{
    return GetString(IDS_SHORTDESC);
}

const TCHAR* Exporter::AuthorName()
{
    return _T("bumpsgoodman");
}

const TCHAR* Exporter::CopyrightMessage()
{
    return GetString(IDS_COPYRIGHT);
}

const TCHAR* Exporter::OtherMessage1()
{
    return _T("");
}

const TCHAR* Exporter::OtherMessage2()
{
    return _T("");
}

unsigned int Exporter::Version()
{
    return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg,
                                        WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        CenterWindow(hWnd, GetParent(hWnd));
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hWnd, 1);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

void Exporter::ShowAbout(HWND hWnd)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

int	Exporter::DoExport(const TCHAR* pName, ExpInterface* pExpInterface, Interface* pInterface, BOOL bSuppressPrompts, DWORD options)
{
    m_pInterface = pInterface;

    m_pFile = _wfopen(pName, L"wb");
    if (m_pFile == nullptr)
    {
        return IMPEXP_FAIL;
    }

    // 진행 표시줄 시작
    m_pInterface->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, nullptr, nullptr);

    m_numCurNode = 0;
    m_numTotalNode = 0;

    // 총 노드 수 계산
    int numChildren = m_pInterface->GetRootNode()->NumberOfChildren();
    for (int idx = 0; idx < numChildren; idx++) {
        if (pInterface->GetCancel())
        {
            break;
        }

        calcNumTotalNodeRecursion(m_pInterface->GetRootNode()->GetChildNode(idx));
    }

    for (int idx = 0; idx < numChildren; idx++)
    {
        if (m_pInterface->GetCancel())
        {
            break;
        }

        exportNodeRecursion(m_pInterface->GetRootNode()->GetChildNode(idx));
    }

    // 진행 표시줄 마침
    m_pInterface->ProgressEnd();

    fclose(m_pFile);

    return IMPEXP_SUCCESS;
}

BOOL Exporter::SupportsOptions(int ext, DWORD options)
{
    return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
}

TCHAR* GetString(int id)
{
    static TCHAR buf[256];

    if (g_hInstance)
    {
        return LoadString(g_hInstance, id, buf, _countof(buf)) ? buf : NULL;
    }

    return NULL;
}

void Exporter::calcNumTotalNodeRecursion(INode* pNode)
{
    ++m_numTotalNode;

    for (int i = 0; i < pNode->NumberOfChildren(); ++i)
    {
        calcNumTotalNodeRecursion(pNode->GetChildNode(i));
    }
}

void Exporter::exportNodeRecursion(INode* pNode)
{
    ++m_numCurNode;
    m_pInterface->ProgressUpdate((int)((float)m_numCurNode / m_numTotalNode * 100.0f));

    if (m_pInterface->GetCancel())
    {
        return;
    }

    // 객체에 대한 모든 정보..?
    ObjectState os = pNode->EvalWorldState(0);
    if (os.obj != nullptr)
    {
        switch (os.obj->SuperClassID())
        {
        case GEOMOBJECT_CLASS_ID:
            exportGeomObject(pNode);
            break;
        default:
            break;
        }
    }

    for (int i = 0; i < pNode->NumberOfChildren(); ++i)
    {
        exportNodeRecursion(pNode->GetChildNode(i));
    }
}

void Exporter::exportGeomObject(INode* pNode)
{
    Matrix3 tm = pNode->GetObjTMAfterWSM(0);
    const DWORD wireColor = pNode->GetWireColor();

    ObjectState os = pNode->EvalWorldState(0);
    if (os.obj == nullptr)
    {
        return;
    }

    bool bDeleteTri = false;
    Object* pObj = os.obj;
    TriObject* pTri = nullptr;
    if (pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
    {
        pTri = (TriObject*)pObj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));

        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObj != pTri)
        {
            bDeleteTri = true;
        }
    }

    Mesh* pMesh = &pTri->GetMesh();

    UW3D uw3d = {};
    strcpy(uw3d.Magic, "uw3d");

    uw3d.NumVertices = pMesh->getNumVerts();
    uw3d.NumIndices = pMesh->getNumFaces();
    uw3d.WireframeColor.Red = GetRValue(wireColor) / 255.0f;
    uw3d.WireframeColor.Green = GetGValue(wireColor) / 255.0f;
    uw3d.WireframeColor.Blue = GetBValue(wireColor) / 255.0f;
    uw3d.WireframeColor.Alpha = 1.0f;

    fwrite(&uw3d, sizeof(UW3D), 1, m_pFile);

    float vertices[3];
    float colors[4];
    for (DWORD i = 0; i < uw3d.NumVertices; ++i)
    {
        Point3 p = tm * pMesh->verts[i];
        Point3 vc = pMesh->vertCol[i];
        vertices[0] = p.x;
        vertices[1] = p.z;
        vertices[2] = p.y;

        colors[0] = vc.x;
        colors[1] = vc.y;
        colors[2] = vc.z;
        colors[3] = 1.0f;
        fwrite(vertices, sizeof(vertices), 1, m_pFile);
        fwrite(colors, sizeof(colors), 1, m_pFile);
    }

    WORD indices[3];
    for (DWORD i = 0; i < uw3d.NumIndices; ++i)
    {
        indices[0] = (WORD)pMesh->faces[i].v[0];
        indices[1] = (WORD)pMesh->faces[i].v[2];
        indices[2] = (WORD)pMesh->faces[i].v[1];

        fwrite(indices, sizeof(indices), 1, m_pFile);
    }

    if (bDeleteTri)
    {
        delete pTri;
    }
}