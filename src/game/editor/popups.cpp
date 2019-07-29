/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/tl/array.h>

#include <engine/console.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/storage.h>

#include "editor.h"

void CEditor::UiDoPopupMenu()
{

}

int CEditor::PopupGroup(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupLayer(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupQuad(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupPoint(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupNewFolder(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupMapInfo(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupEvent(CEditor *pEditor)
{
	return 0;
}


static int g_SelectImageSelected = -100;
static int g_SelectImageCurrent = -100;

int CEditor::PopupSelectImage(CEditor *pEditor)
{
	return 0;
}

void CEditor::PopupSelectImageInvoke(int Current, float x, float y)
{

}

int CEditor::PopupSelectImageResult()
{
	if(g_SelectImageSelected == -100)
		return -100;

	g_SelectImageCurrent = g_SelectImageSelected;
	g_SelectImageSelected = -100;
	return g_SelectImageCurrent;
}

static int s_GametileOpSelected = -1;

int CEditor::PopupSelectGametileOp(CEditor *pEditor)
{
	return 0;
}

void CEditor::PopupSelectGametileOpInvoke(float x, float y)
{

}

int CEditor::PopupSelectGameTileOpResult()
{
	if(s_GametileOpSelected < 0)
		return -1;

	int Result = s_GametileOpSelected;
	s_GametileOpSelected = -1;
	return Result;
}

int CEditor::PopupDoodadAutoMap(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupSelectDoodadRuleSet(CEditor *pEditor)
{
	return 0;
}

int CEditor::PopupSelectConfigAutoMap(CEditor *pEditor)
{
	return 0;
}

void CEditor::PopupSelectConfigAutoMapInvoke(float x, float y)
{

}

bool CEditor::PopupAutoMapProceedOrder()
{
	return false;
}

int CEditor::PopupColorPicker(CEditor *pEditor)
{
	return 0;
}
