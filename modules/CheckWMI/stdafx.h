/**************************************************************************
*   Copyright (C) 2004-2007 by Michael Medin <michael@medin.name>         *
*                                                                         *
*   This code is part of NSClient++ - http://trac.nakednuns.org/nscp      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#pragma once


//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#include <windows.h>


#include <comdef.h>
#include <Wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")
//# pragma comment(lib, "credui.lib")
# pragma comment(lib, "comsuppw.lib")



#include <string>
#include <functional>

#define _WIN32_DCOM
#include <objbase.h>

#include <common.hpp>
#include <NSCAPI.h>
#include <nscapi/plugin.hpp>
#include <nscapi/nscapi_plugin_wrapper.hpp>
#include <checkHelpers.hpp>
#include <filter_framework.hpp>

#include <atlbase.h>
//#include <atlcom.h>
//#include <atlstr.h>
#include <atlsafe.h>


#ifdef MEMCHECK
#include <vld.h>
#endif
