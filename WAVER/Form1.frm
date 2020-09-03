VERSION 5.00
Begin VB.Form Form1 
   BackColor       =   &H00000000&
   BorderStyle     =   0  'None
   Caption         =   "Form1"
   ClientHeight    =   8580
   ClientLeft      =   1140
   ClientTop       =   1515
   ClientWidth     =   9825
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   572
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   655
   ShowInTaskbar   =   0   'False
   WindowState     =   2  'Maximized
   Begin VB.CommandButton Command1 
      Caption         =   "Esci!!!"
      Height          =   375
      Left            =   3720
      TabIndex        =   1
      Top             =   6960
      Width           =   1815
   End
   Begin VB.Timer Timer1 
      Enabled         =   0   'False
      Interval        =   1
      Left            =   1200
      Top             =   240
   End
   Begin VB.PictureBox Picture1 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ClipControls    =   0   'False
      ForeColor       =   &H80000008&
      Height          =   3600
      Left            =   2400
      Picture         =   "Form1.frx":0000
      ScaleHeight     =   240
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   320
      TabIndex        =   0
      Top             =   600
      Width           =   4800
   End
   Begin VB.Label Label3 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Velocità :"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3600
      TabIndex        =   2
      Top             =   120
      Width           =   2535
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim V As Single

Private Sub Command1_Click()
    End
End Sub

Private Sub Form_Load()
    Dim W, H
    
    V = 4
    
    Picture1.Top = 600
    Picture1.AutoSize = True
    Show
    DoEvents
    
    W = Picture1.Width
    H = Picture1.Height
    hMemDC = CreateCompatibleDC(Picture1.hdc)
    hMemBitmap = CreateCompatibleBitmap(Picture1.hdc, W, H)
    hOldMemBitmap = SelectObject(hMemDC, hMemBitmap)
    ErrCode = Rectangle(hMemDC, 0, 0, W, H)

    
    ErrCode = StretchBlt(hMemDC, 0, 0, 320, 240, Picture1.hdc, 0, 0, W, H, &HCC0020)
    Timer1.Enabled = True
End Sub


Private Sub Timer1_Timer()
    Dim X As Integer
    Dim Y As Integer
    Dim A As Single
    Dim B As Single
    Dim Deg As Integer
    Dim Rad As Single
    Dim Cycle As Integer
    Dim I As Integer
    
    Label3.Caption = "Velocità: " & V
    
    Picture1.Picture = Nothing
    Picture1.AutoRedraw = False
    Timer1.Enabled = False
    
    Deg = 0
    X = 0
    B = 0
    Cycle = 0
    Y = 240
    A = 0
    Do
        Rad = Deg * (3.141592 / 180)
        ErrCode = StretchBlt(Form1.hdc, 160 + X + Cos(Rad) * A, 120 + Sin(Rad) * A, B, 240, hMemDC, X, 0, V, 240, &HCC0020)
        X = X + V
        Deg = Deg + V * 2
        If X >= 320 Then
            X = 0: Cycle = Cycle + 1: Deg = 0 + Cycle * 10
            If A < B Then A = A + 0.01
            If B < V Then B = B + 0.5
        End If
        DoEvents
    Loop
End
    Form1.Cls
    ErrCode = StretchBlt(Form1.hdc, 160, 120, 320, 240, hMemDC, 0, 0, 320, 240, &HCC0020)
    Beep
    
    Picture1.AutoRedraw = True
    Picture1.Width = 320
    Picture1.Height = 240
    
    For Y = 0 To 255
        Picture1.BackColor = RGB(255 - Y, 255 - Y, 255 - Y)
        ErrCode = StretchBlt(Form1.hdc, 160, 120, 320, 240, Picture1.hdc, 0, 0, 320, 240, &H8800C6)
    Next Y
    
    End
End Sub


