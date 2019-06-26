/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#include <engine/console.h>
#include <engine/shared/config.h>

#include "countryflags.h"


void CCountryFlags::LoadCountryflagsIndexfile()
{

}

void CCountryFlags::OnInit()
{

}

int CCountryFlags::Num() const
{
	return 0;
}

const CCountryFlags::CCountryFlag *CCountryFlags::GetByCountryCode(int CountryCode) const
{
	return NULL;
}

const CCountryFlags::CCountryFlag *CCountryFlags::GetByIndex(int Index, bool SkipBlocked) const
{
	return NULL;
}

void CCountryFlags::Render(int CountryCode, const vec4 *pColor, float x, float y, float w, float h, bool AllowBlocked)
{

}
