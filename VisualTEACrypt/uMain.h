//---------------------------------------------------------------------------

#ifndef uMainH
#define uMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

class TfrmMain : public TForm
{
__published:	// IDE-managed Components
    TEdit *edSource;
    TLabel *lblTitle;
    TLabel *lblSourceFile;
    TButton *btnBrowseSource;
    TOpenDialog *odMain;
    TLabel *lblDestFile;
    TEdit *edDest;
    TButton *btnBrowseDest;
    TLabel *lblSourcePath;
    TLabel *lblDestPath;
    TButton *btnStart;
    TButton *btnClose;
    TRadioButton *rbCrypt;
    TRadioButton *rbDecrypt;
    TLabel *Label1;
    TLabel *Label2;
    TEdit *edCycles;
    TLabel *Label3;
    TEdit *edKey;
    TButton *btnBrowseKey;
    TLabel *lblKeyPath;
    TUpDown *udCycles;
    TLabel *Label5;
    TCheckBox *cbVerbose;
    TMainMenu *mmMain;
    TMenuItem *Help1;
    TMenuItem *Help2;
    TMenuItem *Index1;
    TMenuItem *Contents1;
    TMenuItem *N1;
    TMenuItem *Info1;
    TShape *Shape1;
    TPanel *Panel1;
    TRadioButton *rbKeyManual;
    TRadioButton *rbKeyOnFile;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall btnBrowseSourceClick(TObject *Sender);
    void __fastcall btnBrowseDestClick(TObject *Sender);
    void __fastcall btnStartClick(TObject *Sender);
    void __fastcall btnCloseClick(TObject *Sender);
    void __fastcall edSourceChange(TObject *Sender);
    void __fastcall edDestChange(TObject *Sender);
    void __fastcall edDestKeyPress(TObject *Sender, char &Key);
    void __fastcall edSourceKeyPress(TObject *Sender, char &Key);
    void __fastcall rbCryptClick(TObject *Sender);
    void __fastcall rbDecryptClick(TObject *Sender);
    void __fastcall cbVerboseClick(TObject *Sender);
    void __fastcall btnBrowseKeyClick(TObject *Sender);
    void __fastcall edSourceKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall edSourceExit(TObject *Sender);
    void __fastcall edDestKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall edDestExit(TObject *Sender);
    void __fastcall edKeyExit(TObject *Sender);
    void __fastcall edKeyKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall edCyclesKeyPress(TObject *Sender, char &Key);
    void __fastcall rbKeyManualClick(TObject *Sender);
    void __fastcall rbKeyOnFileClick(TObject *Sender);
private:	// User declarations
    void CheckInputTextBox(void);
    int  ProcessFile(void);
    void FreeUsedMem(void);
    void ShowLog();
    void LogInfo(void);
    void errdesc(int errnum);
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
