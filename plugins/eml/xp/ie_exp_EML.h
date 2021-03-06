/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_EXP_EML_H
#define IE_EXP_EML_H

#include "ie_exp_Text.h"

class IE_Exp_EML : public IE_Exp_Text
{
public:
	IE_Exp_EML(PD_Document * pDocument);
	virtual ~IE_Exp_EML();
protected:
	virtual UT_Error _writeDocument(void) override;
};

class IE_Exp_EML_Sniffer : public IE_ExpSniffer
{
public:
	IE_Exp_EML_Sniffer ();
	virtual bool recognizeSuffix(const char * szSuffix) override;
	virtual UT_Error constructExporter(PD_Document * pDocument, IE_Exp ** ppie) override;
	virtual bool getDlgLabels(const char ** pszDesc, const char ** pszSuffixList, IEFileType * ft) override;
};

#endif
