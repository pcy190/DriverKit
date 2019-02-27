
// DriverLoaderDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "DriverLoader.h"

#include "afxdialogex.h"
#include<Winsvc.h>
#include<Windows.h>
#include<wchar.h>
#include<string>
#include "DriverLoaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using std::wstring;
CListBox m_ListMSG;
wstring DriverName;
void Msg(TCHAR *p);
void Msg(CString msg);
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDriverLoaderDlg 对话框



CDriverLoaderDlg::CDriverLoaderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DRIVERLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDriverLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MSG, m_ListMSG);
}

BEGIN_MESSAGE_MAP(CDriverLoaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON_INSTALL, &CDriverLoaderDlg::OnBnClickedButtonInstall)
	ON_BN_CLICKED(IDC_BUTTON_START, &CDriverLoaderDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDriverLoaderDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_UNINSTALL, &CDriverLoaderDlg::OnBnClickedButtonUninstall)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

// 限制窗口最小大小
void CDriverLoaderDlg::OnGetMinMaxInfo(MINMAXINFO * lpMMI)
{
	lpMMI->ptMinTrackSize.x = 600;
	lpMMI->ptMinTrackSize.y = 400;
	//lpMMI->ptMaxTrackSize.x = 1366;
	//lpMMI->ptMaxTrackSize.y = 768;
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

// CDriverLoaderDlg 消息处理程序
BOOL CDriverLoaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	Msg(_T("初始化"));
	memset(szFilePath, 0, sizeof(szFilePath));
	DragAcceptFiles(TRUE);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);//解决Win10下的拖放文件问题
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDriverLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDriverLoaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDriverLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDriverLoaderDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	DragQueryFile(hDropInfo, 0, szFilePath, sizeof(szFilePath)); //获取文件路径
	SetDlgItemText(IDC_FileName, szFilePath);                 //更新文本框
	Msg(szFilePath);
	DragFinish(hDropInfo);                                    //拖放结束后,释放内存 
	CDialogEx::OnDropFiles(hDropInfo);
}

void Msg(TCHAR *p) {
	m_ListMSG.AddString(CString(p));
}
void Msg(CString msg) {
	m_ListMSG.AddString(msg);
}

void CDriverLoaderDlg::OnBnClickedButtonInstall()
{

	if (wcslen(szFilePath)==0)
	{
		Msg(_T("请选择文件"));
		return;
	}
	size_t pos = wstring(szFilePath).find_last_of('\\');
	DriverName.assign(wstring(szFilePath).substr(pos + 1));

	sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!sh)
	{
		Msg(_T("打开服务控制器失败,请检查是否以管理员权限运行"));
		CloseServiceHandle(sh);
		return;
	}
	Msg(_T("Loading----"));
	Msg(CString(DriverName.c_str()));
	SC_HANDLE m_hServiceDDK = CreateService(
		sh,//SMC句柄
		DriverName.c_str(),//驱动服务名称(驱动程序的在注册表中的名字)
		DriverName.c_str(),//驱动服务显示名称(注册表驱动程序的DisplayName值)
		SERVICE_ALL_ACCESS,//权限(所有访问权限)
		SERVICE_KERNEL_DRIVER,//服务类型(驱动程序)
		SERVICE_DEMAND_START,//启动方式(需要时启动,注册表驱动程序的Start值)
		SERVICE_ERROR_IGNORE,//错误控制(忽略,注册表驱动程序的ErrorControl值)
		szFilePath,//服务的二进制文件路径(驱动程序文件路径, 注册表驱动程序的ImagePath值)
		NULL,//加载组命令
		NULL,//TagId
		NULL,//依存关系
		NULL,//服务启动名
		NULL);//密码
	if (!m_hServiceDDK)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			Msg(_T("驱动已经存在"));
			if(!m_hServiceDDK)m_hServiceDDK = OpenService(sh, DriverName.c_str(), SERVICE_ALL_ACCESS);
		}
		else {
			TCHAR msg[100];
			wprintf_s(msg, "安装失败，错误码 %p", GetLastError());
			Msg(msg);

			Msg(_T("Error while Install ,error code:" + GetLastError()));
			MessageBox(NULL, DriverName.c_str(),  MB_OK);
		}
		
	}
	else {
		Msg(_T("驱动安装成功!"));
	}
	CloseServiceHandle(sh);
	CloseServiceHandle(m_hServiceDDK);
}


using namespace std;
void CDriverLoaderDlg::OnBnClickedButtonStart()
{
	if (!m_hServiceDDK)
	{
		Msg(_T("请先安装驱动"));
		return;
	}
	sh= OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!sh)
	{
		Msg(_T("打开服务控制器失败,请检查是否以管理员权限运行"));
		CloseServiceHandle(sh);
		return;
	}
	Msg(_T("启动中"));
	m_hServiceDDK = OpenService(sh, DriverName.c_str(), SERVICE_START);

	if (!StartService(m_hServiceDDK, NULL, NULL))
	{
		DWORD ErrorCode = GetLastError();
		switch (ErrorCode)
		{
		case ERROR_SERVICE_ALREADY_RUNNING:
			Msg(_T("驱动已经运行"));
			break;
		case ERROR_SERVICE_NOT_FOUND:
			Msg(_T("驱动未找到"));
			break;
		default:
			CString msg("启动失败，错误码 %d "+ErrorCode);
			Msg(msg);
			break;
		}
		Msg(_T("启动失败"));
	}
	else {
		Msg(_T("启动成功"));
	}
	CloseServiceHandle(sh);
	CloseServiceHandle(m_hServiceDDK);
}


void CDriverLoaderDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	sh=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	m_hServiceDDK = OpenService(sh, DriverName.c_str(), SERVICE_STOP);
	SERVICE_STATUS svcsta = { 0 };
	BOOL bRet = ControlService(m_hServiceDDK, SERVICE_CONTROL_STOP, &svcsta);
	if (!bRet)
	{
		DWORD ErrorCode = GetLastError();
		switch (ErrorCode)
		{
		case ERROR_SERVICE_NEVER_STARTED:
		case ERROR_SERVICE_NOT_ACTIVE:
			Msg(_T("驱动未启动"));
			break;
		case ERROR_SERVICE_NOT_FOUND:
			Msg(_T("驱动不存在"));
			break;
		default:
			CString msg;
			wprintf_s(msg, "停止失败，错误码 %d", ErrorCode);
			Msg(msg);
			break;
		}
	}
	else {
		Msg(_T("驱动已停止"));
	}
	CloseServiceHandle(sh);
	CloseServiceHandle(m_hServiceDDK);
}


void CDriverLoaderDlg::OnBnClickedButtonUninstall()
{
	sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	m_hServiceDDK = OpenService(sh, DriverName.c_str(), SERVICE_STOP | DELETE);
	if (!DeleteService(m_hServiceDDK))
	{
		DWORD ErrorCode = GetLastError();
		switch (ErrorCode)
		{
		case ERROR_SERVICE_DOES_NOT_EXIST:
		case ERROR_SERVICE_NOT_FOUND:
			Msg(_T("驱动不存在"));
			break;
		default:
			CString msg;
			wprintf_s(msg, "停止失败，错误码 %d", ErrorCode);
			Msg(msg);
			break;
		}
	}
	else {
		Msg(_T("卸载驱动成功"));
	}
	CloseServiceHandle(sh);
	CloseServiceHandle(m_hServiceDDK);
}
