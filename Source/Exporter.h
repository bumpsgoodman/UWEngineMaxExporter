/*
* 3DS Max Exporter
*
* 작성자: bumpsgoodman
* 작성일: 2023.01.31
*/

#pragma once

#include "max.h"
#include "resource.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"

#include "istdplug.h"

#define CFGFILENAME		_T("UW3DEXPORTER.CFG")	// Configuration file

extern ClassDesc* GetUW3dExpDesc();
extern TCHAR* GetString(int id);
extern HINSTANCE g_hInstance;

class Exporter final : public SceneExport
{
public:
    Exporter() = default;
    Exporter(const Exporter&) = delete;
    Exporter& operator=(const Exporter&) = delete;
    Exporter(Exporter&&) = default;
    Exporter& operator=(Exporter&&) = default;
    ~Exporter();

    // SceneExport 메서드
    int ExtCount();                     // 지원되는 확장자 수
    const TCHAR* Ext(int n);            // 확장자
    const TCHAR* LongDesc();            // 긴 설명
    const TCHAR* ShortDesc();           // 짧은 설명
    const TCHAR* AuthorName();          // 작성자
    const TCHAR* CopyrightMessage();    // 저작권 메시지
    const TCHAR* OtherMessage1();       // Other message #1
    const TCHAR* OtherMessage2();       // Other message #2
    unsigned int Version();             // 버전 번호
    void ShowAbout(HWND hWnd);          // DLL About 박스
    int	DoExport(const TCHAR* pName, ExpInterface* pExpInterface, Interface* pInterface, BOOL bSuppressPrompts = FALSE, DWORD options = 0); // Export 실행 함수
    BOOL SupportsOptions(int ext, DWORD options);

private:
    void calcNumTotalNodeRecursion(INode* pNode);
    void exportNodeRecursion(INode* pNode);
    void exportGeomObject(INode* pNode);

private:
    int m_numTotalNode = 0;
    int m_numCurNode = 0;
    Interface* m_pInterface = nullptr;

    FILE* m_pFile = nullptr;
};