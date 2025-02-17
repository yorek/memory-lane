//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("VisualTEACrypt.res");
USEFORM("uMain.cpp", frmMain);
USEUNIT("uTEA.cpp");
USEUNIT("uCRC32.cpp");
USEFORM("uLog.cpp", frmLog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
         Application->Initialize();
         Application->Title = "Visual TEACrypt 80x86";
         Application->CreateForm(__classid(TfrmMain), &frmMain);
         Application->CreateForm(__classid(TfrmLog), &frmLog);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
