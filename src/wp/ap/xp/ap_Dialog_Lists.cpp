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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ap_Dialog_Lists.h"
#include "ap_Strings.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"


AP_Dialog_Lists::AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory, id)
{
  	m_pView = NULL;
	m_answer = a_CLOSE;
        m_bStartList = UT_FALSE;
        m_bStopList = UT_FALSE;
	m_bChangeStartValue = UT_FALSE;
}

AP_Dialog_Lists::~AP_Dialog_Lists(void)
{
}

AP_Dialog_Lists::tAnswer AP_Dialog_Lists::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}


void  AP_Dialog_Lists::StartList(void)
{
       getBlock()->listUpdate();
       getView()->cmdStartList(getListStyleString());
}


void  AP_Dialog_Lists::StopList(void)
{
       getBlock()->listUpdate();
       getView()->cmdStopList();
}

XML_Char* AP_Dialog_Lists::getListStyleString(void)
{
       XML_Char* style;

       // These strings match piece table styles and should not be 
       // internationalized

       switch (m_iListType)
       {
       case 0:
	      style = (XML_Char *)  "Numbered List";
	      break;
       case 1:
     	      style = (XML_Char *)  "Lower Case List";
	      break;
       case 2:
	      style = (XML_Char *)  "Upper Case List"; 
	      break;
       case 3:
	      style = (XML_Char *)  "Bullet List";
	      break;
       default:
	      return (XML_Char *) NULL;
       }
       return style;
}

fl_AutoNum * AP_Dialog_Lists::getAutoNum(void)
{
      return getBlock()->getAutoNum();
}


fl_BlockLayout * AP_Dialog_Lists::getBlock(void)
{
      return getView()->getCurrentBlock();
}

void  AP_Dialog_Lists::Apply(void)
{
       XML_Char* style;
       if(m_bChangeStartValue == UT_TRUE)
       {
              getAutoNum()->setAsciiOffset(0);
	      style = getListStyleString();
	      getView()->changeListStyle(getAutoNum(),style);
       	      getAutoNum()->setStartValue(m_curStartValue);
	      getAutoNum()->setFormat(m_newListType);
       	      getAutoNum()->update(0);
	      getBlock()->listUpdate();
	      return;
       }
       if(m_bStopList == UT_TRUE)
       { 
	      StopList();
	      return;
       }
       if(m_bStartList == UT_TRUE)
       { 
	      StartList(); //Start with a numbered list then change the format
	                   //and starting value
	      switch(m_iListType)
	      {
	      case 0:  // Numbered List
	      
		       getAutoNum()->setStartValue(m_newStartValue);
     		       getAutoNum()->setFormat(m_newListType);
		       break;
	      case 1:  // Lowe Case List
		       getAutoNum()->setStartValue(m_newStartValue); 
     		       getAutoNum()->setFormat(m_newListType);
		       break;
	      case 2:  // Upper Case List
		       getAutoNum()->setStartValue(m_newStartValue); 
     		       getAutoNum()->setFormat(m_newListType);
		       break;
	      case 3:  // Bulleted List
		       getAutoNum()->setFormat(m_newListType);
		       break;
	      default:
		       UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      }
	      getBlock()->listUpdate();
	      return;
       }
}

void  AP_Dialog_Lists::PopulateDialogData(void)
{
       const XAP_StringSet * pSS = m_pApp->getStringSet();
       m_isListAtPoint = getBlock()->isListItem();
       if(m_isListAtPoint == UT_TRUE)
       {
	      const char * tmp1 =  (char *) getBlock()->getListLabel();
	      strcpy( m_curListLabel, tmp1);
              m_curListLevel = getBlock()->getLevel();
              m_curStartValue = getAutoNum()->getStartValue32();
	      const char * tmp2 = getAutoNum()->getType();
	      if(strstr(tmp2,"%*%d.")!= NULL)
	      {
		      strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
		      m_iListType = 0;
	      }
	      else if(strstr(tmp2,"%*%a.")!= NULL)
	      {
		      strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
		      m_iListType = 1;
	      }	      
	      else if(strstr(tmp2,"%*%A.")!= NULL)
	      {
		      strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
		      m_iListType = 2;
	      }
	      else if(strstr(tmp2,"%b")!= NULL)
	      {
		      strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
		      m_iListType = 3;
	      }
	      else
	      {
		      strcpy(m_curListType,tmp2);
	      }
       }
}

UT_Bool  AP_Dialog_Lists::isLastOnLevel(void)
{
       return getAutoNum()->isLastOnLevel(getBlock());
}



// --------------------------- Generic Functions -----------------------------


void AP_Dialog_Lists::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
        UT_uint32 title_width = 33;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_Lists_Title));
        BuildWindowName((char *) m_WindowName,tmp,title_width);
        FREEP(tmp);

}

void  AP_Dialog_Lists::setActiveFrame(XAP_Frame *pFrame)
{
        setView(getView());
	notifyActiveFrame(getActiveFrame());
}

UT_Bool AP_Dialog_Lists::setView(FV_View * view)
{
        m_pView =  (FV_View *) getActiveFrame()->getCurrentView();
	return UT_TRUE;
}

FV_View * AP_Dialog_Lists::getView(void)
{
        XAP_Frame * pFrame =  getActiveFrame();
        return  (FV_View *) pFrame->getCurrentView();
}







