//---------------------------------------------------------------------------

#ifndef uLogH
#define uLogH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmLog : public TForm
{
__published:	// IDE-managed Components
    TMemo *mLog;
    TPanel *Panel1;
    TLabel *lblStatus;
    TProgressBar *pbStatus;
    TAnimate *anProgress;
private:	// User declarations
public:		// User declarations
    __fastcall TfrmLog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmLog *frmLog;
//---------------------------------------------------------------------------
#endif
