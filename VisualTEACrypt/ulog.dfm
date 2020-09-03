object frmLog: TfrmLog
  Left = 221
  Top = 248
  BorderIcons = [biSystemMenu, biMinimize, biHelp]
  BorderStyle = bsToolWindow
  Caption = 'frmLog'
  ClientHeight = 321
  ClientWidth = 640
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object mLog: TMemo
    Left = 0
    Top = 0
    Width = 640
    Height = 280
    Align = alClient
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Courier'
    Font.Style = []
    Lines.Strings = (
      '')
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 280
    Width = 640
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    object lblStatus: TLabel
      Left = 8
      Top = 4
      Width = 41
      Height = 13
      AutoSize = False
      Caption = 'Status'
    end
    object pbStatus: TProgressBar
      Left = 32
      Top = 19
      Width = 593
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Min = 0
      Max = 100
      Smooth = True
      TabOrder = 0
    end
    object anProgress: TAnimate
      Left = 8
      Top = 19
      Width = 17
      Height = 17
      Active = False
      AutoSize = False
      CommonAVI = aviFindComputer
      StopFrame = 8
    end
  end
end
