object frmMain: TfrmMain
  Left = 403
  Top = 112
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Visual TEACrypt 80x86 v 1.0 BETA'
  ClientHeight = 362
  ClientWidth = 362
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = mmMain
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object lblSourceFile: TLabel
    Left = 8
    Top = 40
    Width = 64
    Height = 13
    Caption = 'Source File:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object lblDestFile: TLabel
    Left = 8
    Top = 96
    Width = 90
    Height = 13
    Caption = 'Destination File:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object lblSourcePath: TLabel
    Left = 8
    Top = 80
    Width = 345
    Height = 11
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object lblDestPath: TLabel
    Left = 8
    Top = 136
    Width = 345
    Height = 11
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label1: TLabel
    Left = 8
    Top = 152
    Width = 39
    Height = 13
    Caption = 'Action:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label2: TLabel
    Left = 8
    Top = 192
    Width = 39
    Height = 13
    Caption = 'Cycles:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label3: TLabel
    Left = 8
    Top = 240
    Width = 24
    Height = 13
    Caption = 'Key:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lblKeyPath: TLabel
    Left = 8
    Top = 304
    Width = 345
    Height = 11
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 192
    Top = 152
    Width = 47
    Height = 13
    Caption = 'Logging:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Shape1: TShape
    Left = 0
    Top = 0
    Width = 361
    Height = 33
  end
  object lblTitle: TLabel
    Left = 0
    Top = 0
    Width = 362
    Height = 33
    Alignment = taCenter
    AutoSize = False
    Caption = 'lblTitle'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    Transparent = True
    Layout = tlCenter
  end
  object edSource: TEdit
    Left = 8
    Top = 56
    Width = 281
    Height = 21
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
    OnChange = edSourceChange
    OnExit = edSourceExit
    OnKeyPress = edSourceKeyPress
    OnKeyUp = edSourceKeyUp
  end
  object btnBrowseSource: TButton
    Left = 288
    Top = 56
    Width = 65
    Height = 21
    Caption = 'Browse'
    TabOrder = 1
    OnClick = btnBrowseSourceClick
  end
  object edDest: TEdit
    Left = 8
    Top = 112
    Width = 281
    Height = 21
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnChange = edDestChange
    OnExit = edDestExit
    OnKeyPress = edDestKeyPress
    OnKeyUp = edDestKeyUp
  end
  object btnBrowseDest: TButton
    Left = 288
    Top = 112
    Width = 65
    Height = 21
    Caption = 'Browse'
    TabOrder = 3
    OnClick = btnBrowseDestClick
  end
  object btnStart: TButton
    Left = 16
    Top = 328
    Width = 121
    Height = 25
    Caption = 'Start'
    Enabled = False
    TabOrder = 4
    OnClick = btnStartClick
  end
  object btnClose: TButton
    Left = 224
    Top = 328
    Width = 121
    Height = 25
    Caption = 'Close'
    TabOrder = 5
    OnClick = btnCloseClick
  end
  object rbCrypt: TRadioButton
    Left = 8
    Top = 168
    Width = 89
    Height = 17
    Caption = 'Crypt file'
    Checked = True
    TabOrder = 6
    TabStop = True
    OnClick = rbCryptClick
  end
  object rbDecrypt: TRadioButton
    Left = 88
    Top = 168
    Width = 97
    Height = 17
    Caption = 'Decrypt file'
    TabOrder = 7
    OnClick = rbDecryptClick
  end
  object edCycles: TEdit
    Left = 8
    Top = 208
    Width = 49
    Height = 21
    ParentShowHint = False
    ShowHint = True
    TabOrder = 8
    Text = '0'
    OnKeyPress = edCyclesKeyPress
  end
  object edKey: TEdit
    Left = 8
    Top = 280
    Width = 257
    Height = 21
    ParentShowHint = False
    ShowHint = True
    TabOrder = 9
    OnExit = edKeyExit
    OnKeyUp = edKeyKeyUp
  end
  object btnBrowseKey: TButton
    Left = 272
    Top = 280
    Width = 77
    Height = 21
    Caption = 'Browse File'
    Enabled = False
    TabOrder = 10
    OnClick = btnBrowseKeyClick
  end
  object udCycles: TUpDown
    Left = 57
    Top = 208
    Width = 15
    Height = 21
    Associate = edCycles
    Min = 0
    Max = 128
    Position = 0
    TabOrder = 11
    Wrap = False
  end
  object cbVerbose: TCheckBox
    Left = 192
    Top = 168
    Width = 81
    Height = 21
    Caption = 'Be Verbose'
    TabOrder = 12
    OnClick = cbVerboseClick
  end
  object Panel1: TPanel
    Left = 8
    Top = 260
    Width = 257
    Height = 17
    BevelOuter = bvNone
    TabOrder = 13
    object rbKeyManual: TRadioButton
      Left = 0
      Top = 0
      Width = 102
      Height = 17
      Caption = 'Insert manually'
      Checked = True
      TabOrder = 0
      TabStop = True
      OnClick = rbKeyManualClick
    end
    object rbKeyOnFile: TRadioButton
      Left = 104
      Top = 0
      Width = 121
      Height = 17
      Caption = 'Read keys from file'
      TabOrder = 1
      OnClick = rbKeyOnFileClick
    end
  end
  object odMain: TOpenDialog
    DefaultExt = '*.*'
    FileName = '*.*'
    Filter = 'All Files|*.*'
    Options = [ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 304
    Top = 208
  end
  object mmMain: TMainMenu
    Left = 304
    Top = 160
    object Help1: TMenuItem
      Caption = '&Help'
      object Help2: TMenuItem
        Caption = 'Help'
      end
      object Index1: TMenuItem
        Caption = 'Index'
      end
      object Contents1: TMenuItem
        Caption = 'Contents'
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object Info1: TMenuItem
        Caption = 'About'
      end
    end
  end
end
