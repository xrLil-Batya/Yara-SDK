//----------------------------------------------------
// file: CSceneObject.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include "SceneObject.h"
#include "Scene.h"
//----------------------------------------------------
#define SCENEOBJ_CURRENT_VERSION		0x0011
//----------------------------------------------------
#define SCENEOBJ_CHUNK_VERSION		  	0x0900
#define SCENEOBJ_CHUNK_REFERENCE     	0x0902
#define SCENEOBJ_CHUNK_PLACEMENT     	0x0904
#define SCENEOBJ_CHUNK_FLAGS			0x0905
//#define SCENEOBJ_CHUNK_OMOTIONS			0xF914
//#define SCENEOBJ_CHUNK_ACTIVE_OMOTION	0xF915
//#define SCENEOBJ_CHUNK_SOUNDS			0xF920
//#define SCENEOBJ_CHUNK_ACTIVE_SOUND		0xF921
bool CSceneObject::Load(IReader& F)
{
    bool bRes = true;
	do{
        u32 version = 0;
        string1024 buf;
        R_ASSERT(F.r_chunk(SCENEOBJ_CHUNK_VERSION,&version));
        if ((version!=0x0010)&&(version!=SCENEOBJ_CURRENT_VERSION)){
            ELog.DlgMsg( mtError, "CSceneObject: unsupported file version. Object can't load.");
            bRes = false;
            break;
        }

        if (version==0x0010){
	        R_ASSERT(F.find_chunk(SCENEOBJ_CHUNK_PLACEMENT));
    	    F.r_fvector3(FPosition);
	        F.r_fvector3(FRotation);
    	    F.r_fvector3(FScale);
        }

		CCustomObject::Load(F);

        R_ASSERT(F.find_chunk(SCENEOBJ_CHUNK_REFERENCE));
        m_Version	= F.r_s32();
        F.r_s32		(); // advance (old read vers)
        F.r_stringZ	(buf,sizeof(buf));
        if (!SetReference(buf))
        {
            ELog.Msg            ( mtError, "CSceneObject: '%s' not found in library", buf );
            bRes                = false;
            int mr              = mrNone;

            xr_string       _new_name;
            bool b_found    = Scene->GetSubstObjectName(buf, _new_name);
            if(b_found)
            {
                xr_string _message;
                _message = "Object ["+xr_string(buf)+"] not found. Relace it with ["+_new_name+"] or select other from library?";
                mr = ELog.DlgMsg(mtConfirmation,TMsgDlgButtons() << mbYes << mbNo, _message.c_str());
                if(mrYes==mr)
                {
                    bRes = SetReference(_new_name.c_str());
                }
            }
            if(!bRes)
            {
                if(mr == mrNone)
//                    mr = ELog.DlgMsg(mtConfirmation,TMsgDlgButtons() << mbYes << mbNo, "Object not found. Do you want to select it from library?");
					mr = ELog.DlgMsg(mtConfirmation,TMsgDlgButtons() << mbYes << mbNo, "Object [%s] not found. Do you want to select it from library?", buf );
                else
                    mr = mrNone;

                LPCSTR new_val = 0;
                if ( (mr==mrNone||mr==mrYes) && TfrmChoseItem::SelectItem(smObject,new_val,1))
                {
                    bRes = SetReference(new_val);
                    if(bRes)
                        Scene->RegisterSubstObjectName(buf, new_val);
                }
            }

            Scene->Modified();
        }
        if(!CheckVersion())
            ELog.Msg( mtError, "CSceneObject: '%s' different file version!", buf );

        // flags
        if (F.find_chunk(SCENEOBJ_CHUNK_FLAGS)){
        	m_Flags.assign(F.r_u32());
        }

        if (!bRes) break;
    }while(0);

    return bRes;
}

void CSceneObject::Save(IWriter& F){
	CCustomObject::Save(F);

	F.open_chunk	(SCENEOBJ_CHUNK_VERSION);
	F.w_u16			(SCENEOBJ_CURRENT_VERSION);
	F.close_chunk	();

    // reference object version
    F.open_chunk	(SCENEOBJ_CHUNK_REFERENCE); R_ASSERT2(m_pReference,"Empty SceneObject REFS");
    F.w_s32			(m_pReference->Version());
    F.w_s32			(0); // reserved
    F.w_stringZ		(m_ReferenceName);
    F.close_chunk	();

    F.open_chunk	(SCENEOBJ_CHUNK_FLAGS);
	F.w_u32			(m_Flags.flags);
    F.close_chunk	();
}
//----------------------------------------------------


