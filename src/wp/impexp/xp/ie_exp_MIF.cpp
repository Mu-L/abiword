/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_exp_MIF.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include"ut_wctomb.h"

//////////////////////////////////////////////////////////////////
// a private listener class to help us translate the document
// into a MIF stream.  code is at the bottom of this file.
//////////////////////////////////////////////////////////////////

class s_MIF_Listener : public PL_Listener
{
public:
	s_MIF_Listener(PD_Document * pDocument,
		       IE_Exp_MIF * pie);
	virtual ~s_MIF_Listener();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	void				_closeBlock(void);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	
	PD_Document *		m_pDocument;
	IE_Exp_MIF *		m_pie;
	bool				m_bInBlock;
	UT_Wctomb 		m_wctomb;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_MIF::IE_Exp_MIF(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_MIF::~IE_Exp_MIF()
{
}

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

// we use a reference-counted sniffer
static IE_Exp_MIF_Sniffer * m_sniffer = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_MIF_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "MIF Exporter";
	mi->desc = "Export MIF Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Exp::unregisterExporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return isCurrentAbiVersion (major, minor, release) ? 1 : 0;
}

#endif

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_MIF_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".mif"));
}

UT_Error IE_Exp_MIF_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_MIF * p = new IE_Exp_MIF(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_MIF_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "MIF (.mif)";
	*pszSuffixList = "*.mif";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_MIF::_writeDocument(void)
{
	m_pListener = new s_MIF_Listener(m_pDocument,this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;

	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_MIF_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_bInBlock = false;
	return;
}

void s_MIF_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
}

s_MIF_Listener::s_MIF_Listener(PD_Document * pDocument,
			       IE_Exp_MIF * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;

	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a block.
	// when going to a file we should not.
	m_bInBlock = false;
}

s_MIF_Listener::~s_MIF_Listener()
{
	_closeBlock();
}

bool s_MIF_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0
			// TODO decide how to indicate objects in MIF output.
			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				return true;

			case PTO_Field:
				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
#else
			return true;
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_MIF_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeBlock();
			return true;
		}

	case PTX_Block:
		{
			_closeBlock();
			m_bInBlock = true;
			return true;
		}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_MIF_Listener::change(PL_StruxFmtHandle /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_MIF_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_MIF_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}
