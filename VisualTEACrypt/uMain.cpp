//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "uMain.h"
#include "uLog.h"
#include "uMisc.h"
#include "uCRC32.h"
#include "uTEA.h"
#include <time.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TfrmMain *frmMain;
AnsiString sAppTitle;

//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
    /* Write title */
    sAppTitle.printf("%s v%i.%i.%i", APPNAME, MAJOR(VERSION), MINOR(VERSION), RELEASE(VERSION));
    frmMain->Caption = sAppTitle;
    lblTitle->Caption = sAppTitle;

    /* Set defaults */
    appmode = CRYPT;
    verbose = VERBON;
    cycles = 16;

    /* Load defaults to GUI */
    rbCrypt->Enabled = true;
    udCycles->Position = 16;
    cbVerbose->Checked = true;                                                              
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnBrowseSourceClick(TObject *Sender)
{
    if (odMain->Execute()) {
        edSource->Text = ExtractFileName(odMain->FileName);
        edSource->Hint = odMain->FileName;
        lblSourcePath->Caption = ExtractFilePath(odMain->FileName);

        if (ExtractFileExt(odMain->FileName) != ".tcf") {
            edDest->Text = ChangeFileExt(edSource->Text,".tcf");
        } else {
            edDest->Text = ChangeFileExt(edSource->Text,".tdf");
            appmode = DECRYPT;
            rbDecrypt->Checked = true;
        }
        edDest->Hint = edSource->Hint;
        lblDestPath->Caption = lblSourcePath->Caption;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnBrowseDestClick(TObject *Sender)
{
    if (odMain->Execute()) {
        edDest->Text = ExtractFileName(odMain->FileName);
        edDest->Hint = odMain->FileName;
        lblDestPath->Caption = ExtractFilePath(odMain->FileName);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnStartClick(TObject *Sender)
{
    /* Initialize GUI */
    ShowLog();

    /* Set some variables */
    cycles = udCycles->Position;

	/* Assign function pointer */
	if (algotype == OLDALG)	{
		procfun = encipher;
		if (appmode == DECRYPT) procfun = decipher;
	} else {
		procfun = encipher_new;
		if (appmode == DECRYPT) procfun = decipher_new;
	}

    frmLog->anProgress->Active = true;

	/* Process file */
	int result = ProcessFile();

    frmLog->anProgress->Active = false;

    /* Handle any eventual error */
    if (result!=0) errdesc(result);

	/* Free allocated memory */
    FreeUsedMem();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnCloseClick(TObject *Sender)
{
    Application->Terminate();    
}
//---------------------------------------------------------------------------

void TfrmMain::CheckInputTextBox(void)
{
    if ((edSource->Text.Trim()!="") && (edDest->Text.Trim()!="")) {
        btnStart->Enabled = true;
    } else {
        btnStart->Enabled = false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edSourceChange(TObject *Sender)
{
    CheckInputTextBox();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edDestChange(TObject *Sender)
{
    CheckInputTextBox();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edDestKeyPress(TObject *Sender, char &Key)
{
    CheckInputTextBox();      
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edSourceKeyPress(TObject *Sender, char &Key)
{
    CheckInputTextBox();
}
//---------------------------------------------------------------------------

int TfrmMain::ProcessFile(void)
{
    AnsiString sTemp;
    AnsiString sSourceFile, sDestFile, sKeyFile;

    /* Build file paths */
    sSourceFile = Trim(lblSourcePath->Caption + edSource->Text);
    sDestFile   = Trim(lblDestPath->Caption + edDest->Text);
    if (rbKeyOnFile->Checked) {
        sKeyFile    = Trim(lblKeyPath->Caption + edKey->Text);
    } else {
        sKeyFile    = "";
    }

	/*
		TEA is capable of processing 64bit at time
		using 2 long word values (32bit each one
		so totally we have 64bit or 8 bytes)
		This version has been improved to process
		4 packet of 64bit at time.
		So the packet lenght is 8 long words
	*/
	char packet_lenght=8;

	/* We need to keep downwards compatibilty */
	if (algotype == OLDALG) packet_lenght = 2;

	/*
		To speed up processing we can buffer data by
		reading more than 1 packet at time (remember
		that a packet is 8 bytes long).
		"bufferpak" contains how many packets we're going
		to buffer. "bufferlen" contains how memory
		(expressed in long words) we need
	*/
	long bufferpak = 2048;
	long bufferlen = packet_lenght * bufferpak;

	fileheader_t header;
    FILE *src, *dst, *kf;
	long rcount, rtotal, wcount, wtotal, packlen, kcount;
	long flen;
	clock_t tstart, tend;
	float elaps;
	int keylen, step;

	/* Create a crc32 class */
	CRC32 filecrc32;

    /* Initialize values */
	rtotal = wtotal = 0;
	rcount = wcount = kcount = 0;
	packlen = sizeof(unsigned long) * bufferlen;
	keylen = sizeof(unsigned long) * 4;

    /* Initialize key */
    kf = NULL;
	if (!sKeyFile.IsEmpty()) {
		kf=fopen(sKeyFile.c_str(), "rb");
		if (kf==NULL) return -103;
    } else {
        memcpy(key, sKeyFile.c_str(), keylen);
    }

	/* Show Params */
    LogInfo();

	data = (unsigned long *)malloc(packlen);
	if (data == NULL) {
		return -200;
	}

	cpd	= (unsigned long *)malloc(packlen);
	if (cpd == NULL) {
		return -201;
	}

	if (verbose == VERBON) {
		sTemp.printf("Packet size\t: %lu bytes \r\n",	sizeof(unsigned long) * packet_lenght);
		sTemp.cat_printf("Buffer size\t: %lu bytes (%lu KB, %lu Packets)\r\n",	packlen,	packlen / 1024, bufferpak);
		sTemp.cat_printf("Allocated mem\t: %lu bytes (%lu KB)",	packlen*2+1024,	(packlen*2+1024) / 1024);
        frmLog->mLog->Lines->Append(sTemp);
	}

	/* Open source file and get lenght */
	src=fopen(sSourceFile.c_str(), "rb");
	if (src==NULL) return -100;
	fseek(src, 0, SEEK_END);
	flen = ftell(src);
	rewind(src);

	/* Open dest file */
	dst=fopen(sDestFile.c_str(), "wb+");
	if (dst==NULL) {
		fclose(src);
		return -101;
	}

	/* Write or Read header and show operating mode */
	if (appmode == CRYPT)
	{
        frmLog->mLog->Lines->Append("");
		frmLog->mLog->Lines->Append(" === CRYPTING === ");
		/* Calculate crc of original data */
		frmLog->mLog->Lines->Append("- Calculating CRC32");
		crc = 0;
		filecrc32.calculate(src);
		crc = filecrc32.get();

		frmLog->mLog->Lines->Append("- Writing header");
		strcpy(header.name,LABEL);
		header.crc = crc;
		header.filelenght = flen;
		header.version = VERSION;
		header.algver = algotype;
		fwrite(&header, sizeof(header), 1, dst);
	} else {
        frmLog->mLog->Lines->Append("");
		frmLog->mLog->Lines->Append(" === DECRYPTING === ");
		frmLog->mLog->Lines->Append("- Reading header...");
		fread(&header, sizeof(header), 1, src);
		/* Since we are decrypting we must have the orginal file lenght */
		if (strcmp(header.name,LABEL)!=0) {
			_fcloseall();
			return -104;
		}
		flen = header.filelenght;
	}

	/* Show header info */
	if (verbose == VERBON) {
		sTemp.printf("\tLabel\t: %s\r\n", header.name);
		sTemp.cat_printf("\tVersion\t: %i.%i.%i\r\n", MAJOR(header.version), MINOR(header.version), RELEASE(header.version));
		sTemp.cat_printf("\tCRC32\t: 0x%08x\r\n", header.crc);
		sTemp.cat_printf("\tSize\t: %i bytes\r\n", flen);
		sTemp.cat_printf("\tAlgor.\t: type %i", header.algver);
        frmLog->mLog->Lines->Append(sTemp);
	}

	/* Check if right algorithm is used */
	if (header.algver != algotype) {
		_fcloseall();
		return -105;
	}

    /* Setup progress bar */
    frmLog->pbStatus->Min=0;
    frmLog->pbStatus->Max=flen;

	/* Process file */
	printf("- Processing\n");
    tstart=clock();
	while (!feof(src))
	{
		/* Read data from source */
		memset(data, 0, packlen);
		rcount = fread(data, sizeof(char), packlen, src);
		if (ferror(src)) {
			rtotal=-1;
			break;
		}
		rtotal += rcount;
        frmLog->pbStatus->Position = rtotal;

		/* Read keys from file if specified */
		if (kf != NULL) {
			memset(key, 0, keylen);
			kcount = fread(key, sizeof(unsigned long), 4, kf);			
			if (kcount < 4) rewind(kf);
		}

		/* Process data */
		for (step=0; step<bufferlen; step+=packet_lenght)
		{
			(*procfun)(&data[step], &cpd[step], key, cycles);
		}
		

		/* Write the correct number of bytes */
		if (flen>rtotal) 
		{ 
			wcount = fwrite(cpd, sizeof(char), packlen, dst);			
			wtotal += wcount;		
		} else {
			wcount = fwrite(cpd, sizeof(char), packlen-(rtotal-flen), dst);			
			wtotal += wcount;		
			break;
		}	
	}
	tend=clock();

	if (appmode == DECRYPT) 
	{
		/* Calculate crc of original data */
		frmLog->mLog->Lines->Append("- Calculating CRC32...");
		crc = 0;
		filecrc32.calculate(dst);
		crc = filecrc32.get();

		if (verbose == VERBON) {
			sTemp.printf("\tCRC32\t: 0x%08x", crc);
            frmLog->mLog->Lines->Append(sTemp);
		}
	}

	_fcloseall();

    /* There was an error during file reading */
	if (rtotal<0) return -102;

	frmLog->mLog->Lines->Append("- Finished");
    frmLog->mLog->Lines->Append("");
	frmLog->mLog->Lines->Append(" === STATISTICS ===");

	/* Statistics */
	elaps = (float)(tend-tstart) / (float)CLOCKS_PER_SEC;
	sTemp.printf("Processed\t: %lu bytes\r\n", wtotal);
	sTemp.cat_printf("Time taken\t: %0.3f secs\r\n", elaps);
	sTemp.cat_printf("Speed\t\t: %0.3f Kb/s", ((float)wtotal / elaps) / 1024.0f);
	sTemp.cat_printf(" (%0.3f Mb/s)", (((float)wtotal / elaps) / 1024.0f) / 1024.f);
    frmLog->mLog->Lines->Append(sTemp);

	if (crc != header.crc)
	{
        frmLog->mLog->Lines->Append("");
		frmLog->mLog->Lines->Append("FILE DECRYPTION FAILURE!");
		return -900;
	}

    frmLog->mLog->Lines->Append("");
	frmLog->mLog->Lines->Append("FILE PROCESSING SUCCEDED!");

	return 0;
}
//---------------------------------------------------------------------------

void TfrmMain::FreeUsedMem(void)
{
	if (data != NULL) free(data);
	if (cpd != NULL) free(cpd);
}
//---------------------------------------------------------------------------

void TfrmMain::ShowLog()
{
    frmLog->Show();
    frmLog->Caption = sAppTitle;
    frmLog->mLog->Lines->Clear();
    frmLog->Update();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::rbCryptClick(TObject *Sender)
{
    if (rbCrypt->Checked) appmode = CRYPT;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::rbDecryptClick(TObject *Sender)
{
    if (rbDecrypt->Checked) appmode = DECRYPT;
}
//---------------------------------------------------------------------------

void TfrmMain::LogInfo(void)
{
    AnsiString sTemp;

    frmLog->mLog->Lines->Append(" === FILE  INFO === ");
    frmLog->mLog->Lines->Append("Source File\t: " + edSource->Text);
    frmLog->mLog->Lines->Append("Dest File\t: " + edDest->Text);
    frmLog->mLog->Lines->Append("Cycles\t\t: " + IntToStr(cycles));
	if (rbKeyManual->Checked) {
		sTemp.printf("Keys\t\t: 0x%08x, 0x%08x, 0x%08x, 0x%08x", key[0], key[1], key[2], key[3]);
        frmLog->mLog->Lines->Append(sTemp);
	} else {
        frmLog->mLog->Lines->Append("Keyfile\t\t: " + edKey->Text);
    }
    frmLog->mLog->Lines->Append("Algorithm\t: type " + IntToStr(algotype));
}


void __fastcall TfrmMain::cbVerboseClick(TObject *Sender)
{
    if (cbVerbose->Checked) {
        verbose = VERBON;
    } else {
        verbose = VERBOFF;
    }
}
//---------------------------------------------------------------------------

void TfrmMain::errdesc(int errnum)
{
    frmLog->mLog->Lines->Append("");
    frmLog->mLog->Lines->Append(" ===== ERROR ====== ");
	switch(errnum)
	{
		case -1:	
			{
				frmLog->mLog->Lines->Append("Please specify the file to be processed");
				break;
			}
		case -2:	
			{	
				frmLog->mLog->Lines->Append("Please specify the output file");
				break;
			}
		case -3:	
			{	
				frmLog->mLog->Lines->Append("Command must be specified");
				break;
			}
		case -100:	
			{
				frmLog->mLog->Lines->Append("Source file not found");
				break;
			}
		case -101:	
			{
				frmLog->mLog->Lines->Append("Cannot create destination file");
				break;
			}
		case -102:
			{
				frmLog->mLog->Lines->Append("Error during file reading");
				break;
			}
		case -103:	
			{
				frmLog->mLog->Lines->Append("Key file not found");
				break;
			}
		case -104:	
			{
				frmLog->mLog->Lines->Append("Source file is not a valid TEA crypted file");
				break;
			}
		case -105:	
			{
				frmLog->mLog->Lines->Append("Wrong algorithm version used");
				break;
			}
		case -900:
			{
				frmLog->mLog->Lines->Append("Decrypted file doesn\'t have the expected crc. Probably you used a wrong key");
				break;
			}
		default:
		frmLog->mLog->Lines->Append("Error code unexpected (" + IntToStr(errnum) + ")");
	}
}
void __fastcall TfrmMain::btnBrowseKeyClick(TObject *Sender)
{
    if (odMain->Execute()) {
        edKey->Text = ExtractFileName(odMain->FileName);
        edKey->Hint = odMain->FileName;
        lblKeyPath->Caption = ExtractFilePath(odMain->FileName);
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edSourceKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (ExtractFilePath(edSource->Text) != "") {
        lblSourcePath->Caption = ExtractFilePath(edSource->Text);
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edSourceExit(TObject *Sender)
{
    edSource->Text = ExtractFileName(edSource->Text);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edDestKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (ExtractFilePath(edDest->Text) != "") {
        lblDestPath->Caption = ExtractFilePath(edDest->Text);
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edDestExit(TObject *Sender)
{
    edDest->Text = ExtractFileName(edDest->Text);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edKeyExit(TObject *Sender)
{
    if (rbKeyOnFile->Checked)
        edKey->Text = ExtractFileName(edKey->Text);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edKeyKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (rbKeyOnFile->Checked)
        if (ExtractFilePath(edKey->Text) != "") {
            lblDestPath->Caption = ExtractFilePath(edKey->Text);
        }
}
//---------------------------------------------------------------------------



void __fastcall TfrmMain::edCyclesKeyPress(TObject *Sender, char &Key)
{
    if ((Key < 48) || (Key > 57)) {
        if (Key != 8) Key = 0;
    }
}
//---------------------------------------------------------------------------



void __fastcall TfrmMain::rbKeyManualClick(TObject *Sender)
{
    btnBrowseKey->Enabled = false;    
    edKey->Clear();
    lblKeyPath->Caption = "";   
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::rbKeyOnFileClick(TObject *Sender)
{
    btnBrowseKey->Enabled = true;
    edKey->Clear();
    lblKeyPath->Caption = "";   
}
//---------------------------------------------------------------------------

