// MFCAppUniversityInfoExtractorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApp-UniversityInfoExtractor.h"
#include "MFCApp-UniversityInfoExtractorDlg.h"
#include "afxdialogex.h"
#include "DataStorage.h"
#include "University.h"
#include <vector>
#include <set>
#include "WebCrawler.h"
#include <afxmt.h>
#include <algorithm>
#include <afxcmn.h> // 进度条控件
#include <future>
#include <mutex>
#include <atomic>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCAppUniversityInfoExtractorDlg 对话框

// 构造函数，初始化对话框并加载主图标
CMFCAppUniversityInfoExtractorDlg::CMFCAppUniversityInfoExtractorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPUNIVERSITYINFOEXTRACTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// 数据交换函数，将控件与成员变量绑定
void CMFCAppUniversityInfoExtractorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, listCtrl);           // 绑定列表控件
	DDX_Control(pDX, IDC_EDIT_SEARCH, editSearch);   // 绑定搜索框
	DDX_Control(pDX, IDC_COMBO_TYPE, comboType);     // 绑定类型下拉框
	DDX_Control(pDX, IDC_COMBO_PROPERTY, comboProperty); // 绑定性质下拉框
	DDX_Control(pDX, IDC_COMBO_FEATURE, comboFeature);   // 绑定特色下拉框
	DDX_Control(pDX, IDC_COMBO_SORT, comboSort);         // 绑定排序下拉框
	DDX_Control(pDX, IDC_LABEL_HINT, labelHint);         // 绑定提示标签
	DDX_Control(pDX, IDC_COMBO_LOCATION, comboLocation); // 绑定所在地筛选器
	DDX_Control(pDX, IDC_PROGRESS1, progressBar);        // 绑定进度条控件
	DDX_Control(pDX, IDC_STATIC_PROGRESS_TEXT, progressText); // 绑定进度文本控件
}

// 消息映射表，关联控件事件与处理函数
BEGIN_MESSAGE_MAP(CMFCAppUniversityInfoExtractorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()	// 系统命令消息
	ON_WM_PAINT()      // 绘制消息
	ON_WM_QUERYDRAGICON() // 拖动图标消息
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CMFCAppUniversityInfoExtractorDlg::OnBnClickedButtonRefresh) // 刷新按钮点击
	ON_EN_CHANGE(IDC_EDIT_SEARCH, &CMFCAppUniversityInfoExtractorDlg::OnEnChangeEditSearch)         // 搜索框内容变化
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboType)   // 类型下拉框选择变化
	ON_CBN_SELCHANGE(IDC_COMBO_PROPERTY, &CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboProperty) // 性质下拉框选择变化
	ON_CBN_SELCHANGE(IDC_COMBO_FEATURE, &CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboFeature)   // 特色下拉框选择变化
	ON_CBN_SELCHANGE(IDC_COMBO_SORT, &CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboSort)         // 排序下拉框选择变化
	ON_MESSAGE(WM_USER + 1, &CMFCAppUniversityInfoExtractorDlg::OnRefreshDone) // 刷新完成自定义消息
	ON_STN_CLICKED(IDC_LABEL_SORT, &CMFCAppUniversityInfoExtractorDlg::OnStnClickedLabelSort) // 排序标签点击
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CMFCAppUniversityInfoExtractorDlg::OnListRClick)         // 列表右键菜单
	ON_CBN_SELCHANGE(IDC_COMBO_LOCATION, &CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboLocation) // 所在地下拉框选择变化
	ON_BN_CLICKED(IDC_BUTTON_RESET_FILTERS, &CMFCAppUniversityInfoExtractorDlg::OnBnClickedButtonResetFilters) // 清空筛选按钮
	ON_BN_CLICKED(IDC_CHECK_SHOW_FAVORITES, &CMFCAppUniversityInfoExtractorDlg::OnBnClickedCheckShowFavorites) // 收藏复选框点击
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMFCAppUniversityInfoExtractorDlg::OnListClick) // 列表单击事件
END_MESSAGE_MAP()


// CMFCAppUniversityInfoExtractorDlg 消息处理程序

BOOL CMFCAppUniversityInfoExtractorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


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

	// 设置此对话框的图标。  
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 初始化列表控件
	LONG lStyle = ::GetWindowLong(listCtrl.GetSafeHwnd(), GWL_STYLE);
	lStyle |= LVS_OWNERDATA; // 启用虚拟模式
	::SetWindowLong(listCtrl.GetSafeHwnd(), GWL_STYLE, lStyle);
	listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	listCtrl.DeleteAllItems();
	while (listCtrl.DeleteColumn(0));
	listCtrl.InsertColumn(0, _T(""), LVCFMT_CENTER, 30); 
	listCtrl.InsertColumn(1, _T("名称"), LVCFMT_LEFT, 230);
	listCtrl.InsertColumn(2, _T("所在地"), LVCFMT_LEFT, 80);
	listCtrl.InsertColumn(3, _T("类型"), LVCFMT_LEFT, 80);
	listCtrl.InsertColumn(4, _T("性质"), LVCFMT_LEFT, 90);
	listCtrl.InsertColumn(5, _T("特色"), LVCFMT_LEFT, 80);
	listCtrl.InsertColumn(6, _T("隶属"), LVCFMT_LEFT, 220);
	listCtrl.InsertColumn(7, _T("学校网址"), LVCFMT_LEFT, 250);

	// 初始化筛选器
	comboType.AddString(_T("全部"));
	comboProperty.AddString(_T("全部"));
	comboFeature.AddString(_T("全部"));
	comboType.SetCurSel(0);
	comboProperty.SetCurSel(0);
	comboFeature.SetCurSel(0);
	// 指定特色的五个选项
	comboFeature.ResetContent();
	comboFeature.AddString(_T("全部"));
	comboFeature.AddString(_T("985"));
	comboFeature.AddString(_T("211"));
	comboFeature.AddString(_T("985/211"));
	comboFeature.AddString(_T("无"));
	comboFeature.SetCurSel(0);

	// 初始化排序下拉框
	comboSort.AddString(_T("默认顺序"));
	comboSort.AddString(_T("名称升序"));
	comboSort.AddString(_T("名称降序"));
	comboSort.SetCurSel(0);

	SetWindowText(_T("高校信息查询"));

	// 初始化进度条和文本，默认隐藏
	progressBar.ShowWindow(SW_HIDE);
	progressText.ShowWindow(SW_HIDE);

	// 绑定收藏复选框
	checkShowFavorites.SubclassDlgItem(IDC_CHECK_SHOW_FAVORITES, this);
	checkShowFavorites.SetCheck(showOnlyFavorites);

	// 加载收藏
	LoadFavorites();

	// 自动刷新，如果没有universities.csv则异步刷新
	CFileFind finder;
	if (!finder.FindFile(_T("universities.csv"))) {
		labelHint.SetWindowText(_T("正在抓取网页并刷新学校信息..."));
		// 显示进度条和文本
		progressBar.SetRange(0, 100);
		progressBar.SetPos(0);
		progressBar.ShowWindow(SW_SHOW);
		progressText.SetWindowText(_T("正在抓取数据... (第1页)"));
		progressText.ShowWindow(SW_SHOW);
		// 隐藏列表控件
		listCtrl.EnableWindow(FALSE);
		listCtrl.ShowWindow(SW_HIDE);
		AfxBeginThread(RefreshThreadProc, this);
	} else {
		LoadUniversityData();
		ApplyFilters();
		UpdateListDisplay();
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 处理系统命令
// nID: 命令ID，lParam: 附加参数
void CMFCAppUniversityInfoExtractorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCAppUniversityInfoExtractorDlg::OnPaint()
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
HCURSOR CMFCAppUniversityInfoExtractorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//-----------------------------------------------------------------

void CMFCAppUniversityInfoExtractorDlg::LoadUniversityData()
{
	DataStorage storage;
	allUniversities = storage.loadData();
	// 先清空筛选器内容，防止重复
	comboType.ResetContent();
	comboProperty.ResetContent();
	comboFeature.ResetContent();
	comboLocation.ResetContent(); // 新增所在地筛选器
	// 填充筛选器选项
	std::set<CString> types, properties, features, locations;
	// 遍历所有高校，收集类型、性质、特色、所在地的唯一值，用于下拉筛选框
	for (const auto& u : allUniversities) {
		CString type = CA2T(u.getProperty("类型").c_str());
		CString property = CA2T(u.getProperty("性质").c_str());
		CString location = CA2T(u.getProperty("所在地").c_str());
		// 过滤无意义内容
		if (!type.IsEmpty() && type != _T("------") && type != _T("--") && type != _T("---") && type != _T("无") && type != _T("暂无"))
			types.insert(type);
		if (!property.IsEmpty() && property != _T("------") && property != _T("--") && property != _T("---") && property != _T("无") && property != _T("暂无"))
			properties.insert(property);
		if (!location.IsEmpty() && location != _T("——") && location != _T("------") && location != _T("---") && location != _T("无") && location != _T("暂无"))
			locations.insert(location);
	}
	// 先添加“全部”选项
	comboType.AddString(_T("全部"));
	comboProperty.AddString(_T("全部"));
	comboLocation.AddString(_T("全部")); 
	// 添加所有唯一类型、性质、所在地到下拉框
	for (const auto& t : types) comboType.AddString(t);
	for (const auto& p : properties) comboProperty.AddString(p);
	for (const auto& l : locations) comboLocation.AddString(l);
	// 特色规定五个选项
	comboFeature.ResetContent();
	comboFeature.AddString(_T("全部"));
	comboFeature.AddString(_T("985"));
	comboFeature.AddString(_T("211"));
	comboFeature.AddString(_T("985/211"));
	comboFeature.AddString(_T("无"));
	
	// 默认选中“全部”
	comboType.SetCurSel(0);
	comboProperty.SetCurSel(0);
	comboFeature.SetCurSel(0);
	comboLocation.SetCurSel(0);
	// 更新提示
	CString msg;
	msg.Format(_T("共加载%d所学校"), (int)allUniversities.size());
	labelHint.SetWindowText(msg);
}

void CMFCAppUniversityInfoExtractorDlg::ApplyFilters()
{
	// 获取搜索框内容
	CString searchText;
	editSearch.GetWindowText(searchText);
	searchText.MakeLower(); // 转为小写，便于不区分大小写搜索

	// 获取下拉框当前选中的类型、性质、特色、所在地
	CString type, property, feature, location;
	comboType.GetLBText(comboType.GetCurSel(), type);
	comboProperty.GetLBText(comboProperty.GetCurSel(), property);
	comboFeature.GetLBText(comboFeature.GetCurSel(), feature);
	comboLocation.GetLBText(comboLocation.GetCurSel(), location);

	// 清空筛选结果
	filteredUniversities.clear();

	// 遍历所有高校，按条件筛选
	for (const auto& u : allUniversities) {
		// 获取高校各属性
		CString name = CA2T(u.getName().c_str());
		CString utype = CA2T(u.getProperty("类型").c_str());
		CString unature = CA2T(u.getProperty("性质").c_str());
		CString ufeature = CA2T(u.getProperty("特色").c_str());
		CString ulocation = CA2T(u.getProperty("所在地").c_str());

		// 搜索内容不为空且未命中则跳过
		if (!searchText.IsEmpty() && name.Find(searchText) == -1) continue;
		// 类型筛选
		if (type != _T("全部") && type != utype) continue;
		// 性质筛选
		if (property != _T("全部") && property != unature) continue;
		// 特色筛选
		if (feature != _T("全部")) {
			if (feature == _T("985")) {
				if (!(ufeature == _T("985") || ufeature == _T("985/211"))) continue;
			} else if (feature == _T("211")) {
				if (!(ufeature == _T("211") || ufeature == _T("985/211"))) continue;
			} else if (feature == _T("985/211")) {
				if (ufeature != _T("985/211")) continue;
			} else if (feature == _T("无")) {
				if (ufeature != _T("")) continue;
			}
		}
		// 所在地筛选
		if (location != _T("全部") && location != ulocation) continue;

		// 满足所有条件，加入筛选结果
		filteredUniversities.push_back(u);
	}

	// 开启只显示收藏
	if (showOnlyFavorites) {
		std::vector<University> favs;
		for (const auto& u : filteredUniversities) {
			if (favoriteUniversities.count(u.getName()))
				favs.push_back(u);
		}
		filteredUniversities = favs;
	}

	// 判断是否有筛选条件或搜索内容
	bool isFiltered = !searchText.IsEmpty() || type != _T("全部") || property != _T("全部") || feature != _T("全部") || location != _T("全部");

	// 根据筛选情况设置提示信息
	CString msg;
	if (isFiltered) {
		if (filteredUniversities.empty())
			msg = _T("未搜索到相关学校");
		else
			msg.Format(_T("搜索到%d所学校"), (int)filteredUniversities.size());
	}
	else {
		msg.Format(_T("共加载%d所学校"), (int)allUniversities.size());
	}
	labelHint.SetWindowText(msg);
}

void CMFCAppUniversityInfoExtractorDlg::UpdateListDisplay()
{
	// 将无效内容转为空串
	auto filterInvalid = [](const CString& s) -> CString {
		static const wchar_t* invalids[] = {
			L"------", L"——", L"---", L"--", L"无", L"暂无"
		};	// 定义无效内容列表
		for (const auto& inv : invalids) {
			if (s.Compare(inv) == 0) return L"";
		}	// 如果是无效内容，返回空串
		return s;
	};
	// 规范化网址显示，只保留www.xxx.xxx.cn/com/net格式
	auto normalizeUrl = [](const CString& s) -> CString {
		CString url = s;
		url.Trim();
		// 去除http://和https://
		if (url.Left(7).CompareNoCase(L"http://") == 0) url = url.Mid(7);
		else if (url.Left(8).CompareNoCase(L"https://") == 0) url = url.Mid(8);
		// 只保留www开头到.cn/.com/.net为止
		int wwwPos = url.Find(L"www.");
		if (wwwPos < 0) return L""; // 不是www开头
		url = url.Mid(wwwPos);
		int dotPos = -1;
		CString suffix;
		// 支持常见一级域名
		const wchar_t* suffixes[] = { L".cn", L".com", L".net", L".edu", L".org" };
		for (const auto& suf : suffixes) {
			int pos = url.Find(suf);
			if (pos >= 0 && (dotPos == -1 || pos < dotPos)) {
				dotPos = pos;
				suffix = suf;
			}
		}
		if (dotPos >= 0) {
			// 截断到一级域名后
			url = url.Left(dotPos + suffix.GetLength());
		}
		// 只保留www.xxx.xxx.cn格式
		return url;
	};

	// 排序，0: 默认顺序（CSV文件里的顺序）, 1: 名称升序, 2: 名称降序
	if (sortOrder == 1) {
		std::sort(filteredUniversities.begin(), filteredUniversities.end(), [](const University& a, const University& b) {
			return a.getName() < b.getName();
		});
	} else if (sortOrder == 2) {
		std::sort(filteredUniversities.begin(), filteredUniversities.end(), [](const University& a, const University& b) {
			return a.getName() > b.getName();
		});
	}

	listCtrl.DeleteAllItems(); // 清空列表控件
	int idx = 0;
	for (const auto& u : filteredUniversities) {
		std::string uname = u.getName();
		CString name = CA2T(uname.c_str());
		CString locationRaw = CA2T(u.getProperty("所在地").c_str());
		CString typeRaw = CA2T(u.getProperty("类型").c_str());
		CString propertyRaw = CA2T(u.getProperty("性质").c_str());
		CString featureRaw = CA2T(u.getProperty("特色").c_str());
		CString urlRaw = CA2T(u.getProperty("网址").c_str());
		CString affiliationRaw = CA2T(u.getProperty("隶属").c_str());
		CString location = filterInvalid(locationRaw);
		CString type = filterInvalid(typeRaw);
		CString property = filterInvalid(propertyRaw);
		CString feature = filterInvalid(featureRaw);
		CString url = normalizeUrl(filterInvalid(urlRaw));
		CString affiliation = filterInvalid(affiliationRaw);
		CString fav = favoriteUniversities.count(uname) ? _T("★") : _T("");
		listCtrl.InsertItem(idx, fav); 
		listCtrl.SetItemText(idx, 1, name);
		listCtrl.SetItemText(idx, 2, location);
		listCtrl.SetItemText(idx, 3, type);
		listCtrl.SetItemText(idx, 4, property);
		listCtrl.SetItemText(idx, 5, feature);
		listCtrl.SetItemText(idx, 6, affiliation);
		listCtrl.SetItemText(idx, 7, url);
		++idx;
	}
	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
}

void CMFCAppUniversityInfoExtractorDlg::OnBnClickedButtonRefresh()
{
    labelHint.SetWindowText(_T("正在抓取网页并刷新学校信息..."));
    // 刷新时禁用并隐藏列表控件
    listCtrl.EnableWindow(FALSE);
    listCtrl.ShowWindow(SW_HIDE);
    // 显示进度条和文本
    progressBar.SetRange(0, 100);
    progressBar.SetPos(0);
    progressBar.ShowWindow(SW_SHOW);
    progressText.SetWindowText(_T("正在抓取数据... (第1页)"));
    progressText.ShowWindow(SW_SHOW);
    AfxBeginThread(RefreshThreadProc, this);
}

void CMFCAppUniversityInfoExtractorDlg::OnEnChangeEditSearch()
{
	ApplyFilters();
	UpdateListDisplay();
}

void CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboType()
{
	ApplyFilters();
	UpdateListDisplay();
}
void CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboProperty()
{
	ApplyFilters();
	UpdateListDisplay();
}
void CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboFeature()
{
	ApplyFilters();
	UpdateListDisplay();
}

void CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboSort()
{
	sortOrder = comboSort.GetCurSel();
	ApplyFilters();
	UpdateListDisplay();
}

void CMFCAppUniversityInfoExtractorDlg::OnCbnSelchangeComboLocation()
{
    ApplyFilters();
    UpdateListDisplay();
}

UINT CMFCAppUniversityInfoExtractorDlg::RefreshThreadProc(LPVOID pParam) {
	auto* pDlg = reinterpret_cast<CMFCAppUniversityInfoExtractorDlg*>(pParam);  // 转换对话框指针
	std::atomic<int> finishedPages(0);									        // 已完成的页面数
	pDlg->SetDlgItemText(IDC_BUTTON_REFRESH, _T("刷新中..."));					// 禁用控件
	pDlg->GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_EDIT_SEARCH)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_COMBO_TYPE)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_COMBO_PROPERTY)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_COMBO_FEATURE)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_BUTTON_RESET_FILTERS)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_COMBO_LOCATION)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_COMBO_SORT)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_CHECK_SHOW_FAVORITES)->EnableWindow(FALSE);
	DataStorage storage;
	std::vector<University> data;
	std::set<std::string> uniqueNames; // 用于存储唯一的大学名称，防止重复
	const int concurrency = 5;	// 线程数
	std::mutex dataMutex; // 用于保护 data 和 uniqueNames 的互斥锁

	bool stopFetching = false;
	int currentPage = 1;   // 当前抓取的页面，从1开始

	while (!stopFetching) {
		std::vector<std::future<std::vector<University>>> futures; // 存储异步任务的结果
		// 启动一批并发任务
		for (int i = 0; i < concurrency && !stopFetching; ++i, ++currentPage) {
			futures.push_back(std::async(std::launch::async, [currentPage]() {
				std::string pageUrl = "http://college.gaokao.com/schlist/p" + std::to_string(currentPage) + "/";
				WebCrawler pageCrawler(pageUrl);
				std::string html = pageCrawler.fetchHTML();
				if (html.find("<h3>抱歉，没有找到相关内容</h3>") != std::string::npos) {
					// 返回空表示终止信号
					return std::vector<University>{};
				}
				return pageCrawler.parseUniversities(html);
				}));
		}
		// 处理结果
		for (auto& fut : futures) {
			std::vector<University> pageData = fut.get();
			if (pageData.empty()) {
				stopFetching = true; // 页面无内容，停止后续抓取
				break;
			}
			{
				std::lock_guard<std::mutex> lock(dataMutex);	// 加锁，防止多个线程同时修改 data 和 uniqueNames
				for (const auto& u : pageData) {
					std::string uname = u.getName();
					//	判断该大学名称是否尚未出现过
					if (uniqueNames.find(uname) == uniqueNames.end()) {
						uniqueNames.insert(uname);
						data.push_back(u);
					}	
				}	// 将新抓取的数据合并到主集合中并防止重复
			}
			int pageNum = ++finishedPages;
			CString progress;
			progress.Format(_T("正在抓取数据... (第%d页)"), pageNum);
			pDlg->progressText.SetWindowText(progress);
			pDlg->progressBar.SetPos(pageNum); // 更新进度条
		}
	}
	pDlg->progressText.SetWindowText(_T("抓取完成，正在保存..."));
	storage.saveData(data);						// 保存抓取到的数据
	::PostMessage(pDlg->GetSafeHwnd(), WM_USER + 1, 0, 0); // 发送自定义消息通知刷新完成
	return 0;
}


LRESULT CMFCAppUniversityInfoExtractorDlg::OnRefreshDone(WPARAM, LPARAM)
{
	// 恢复操作控件
    SetDlgItemText(IDC_BUTTON_REFRESH, _T("刷新数据"));
    GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(TRUE);
    GetDlgItem(IDC_EDIT_SEARCH)->EnableWindow(TRUE);
    GetDlgItem(IDC_COMBO_TYPE)->EnableWindow(TRUE);
    GetDlgItem(IDC_COMBO_PROPERTY)->EnableWindow(TRUE); 
    GetDlgItem(IDC_COMBO_FEATURE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_RESET_FILTERS)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_LOCATION)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_SORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_SHOW_FAVORITES)->EnableWindow(TRUE);
    listCtrl.EnableWindow(TRUE);
    listCtrl.ShowWindow(SW_SHOW);
    // 隐藏进度条和文本
    progressBar.ShowWindow(SW_HIDE);
    progressText.ShowWindow(SW_HIDE);
	// 清空筛选器内容
	showOnlyFavorites = false;
	checkShowFavorites.SetCheck(false);
	// 重新加载数据
    LoadUniversityData();
    ApplyFilters();
    UpdateListDisplay();
    CString msg;
    msg.Format(_T("刷新学校信息完成，共加载%d所学校"), (int)allUniversities.size());
    labelHint.SetWindowText(msg);
    return 0;
}

void CMFCAppUniversityInfoExtractorDlg::OnListRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint ptScreen;
    GetCursorPos(&ptScreen); // 获取鼠标的屏幕坐标
    CPoint ptClient = ptScreen;
    listCtrl.ScreenToClient(&ptClient);  // 转换为列表控件内部坐标
    LVHITTESTINFO hitInfo = {0};
    hitInfo.pt = ptClient;
    int row = listCtrl.HitTest(&hitInfo);// 判断点击的是哪一行
    if (row >= 0 && (hitInfo.flags & LVHT_ONITEM)) {	// 判断是否点中了有效的行
        int col = 0;
        CHeaderCtrl* pHeader = listCtrl.GetHeaderCtrl();
        int nColCount = pHeader->GetItemCount();
        int x = ptClient.x;
        int left = 0;
        for (int i = 0; i < nColCount; ++i) {
            int width = listCtrl.GetColumnWidth(i);
            if (x >= left && x < left + width) {
                col = i;
                break;
            }
            left += width;
        }	// 遍历所有列，累加宽度，判断鼠标横坐标落在哪一列的范围内，从而确定列号 col。
        CMenu menu;
        menu.CreatePopupMenu();
		// 创建弹出菜单，添加“复制”和“复制整行”两个选项。
        menu.AppendMenu(MF_STRING, 1, _T("复制"));
        menu.AppendMenu(MF_STRING, 2, _T("复制整行"));	
        // 动态添加收藏/取消收藏
        CString name = listCtrl.GetItemText(row, 1);
        CT2A nameA(name);
        bool isFav = favoriteUniversities.count(nameA.m_psz) > 0;
        if (isFav)
            menu.AppendMenu(MF_STRING, 3, _T("取消收藏"));
        else
            menu.AppendMenu(MF_STRING, 3, _T("收藏"));
        int cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, ptScreen.x, ptScreen.y, this);
        if (cmd == 1 || cmd == 2) {
            CString text;
            if (cmd == 1) {
                text = listCtrl.GetItemText(row, col); // 复制一格
            } 
            else if (cmd == 2) {
                for (int i = 0; i < nColCount; ++i) { // 复制整行，用“,”分隔。
                    if (i > 0) text += _T(",");
                    text += listCtrl.GetItemText(row, i);
                }
            }
            // 复制到剪贴板
            if (!text.IsEmpty()) {
                if (OpenClipboard()) {
                    EmptyClipboard();
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.GetLength() + 1) * sizeof(TCHAR));
                    if (hMem) {
                        memcpy(GlobalLock(hMem), text.GetBuffer(), (text.GetLength() + 1) * sizeof(TCHAR));
                        GlobalUnlock(hMem);
                        SetClipboardData(CF_UNICODETEXT, hMem);
                    }
                    CloseClipboard();
                }
            }
        } else if (cmd == 3) {
            // 收藏/取消收藏
            ToggleFavorite(nameA.m_psz);
            UpdateListDisplay();
        }
    }
    *pResult = 0;	// 通知 MFC 消息已处理。
}

// 消息预处理函数 PreTranslateMessage 的实现
// MFC 对话框默认回车会触发“确定”按钮，导致对话框关闭。
// 因此拦截特点键盘消息，防止用户在搜索框中按下回车键时导致对话框意外关闭
BOOL CMFCAppUniversityInfoExtractorDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        CWnd* pFocus = GetFocus();
        if (pFocus && pFocus->GetDlgCtrlID() == IDC_EDIT_SEARCH) {
            // 阻止回车导致对话框关闭
            return TRUE;
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}

void CMFCAppUniversityInfoExtractorDlg::OnBnClickedButtonResetFilters()
{
	// 重置所有筛选控件为“全部”，清空搜索框
	comboType.SetCurSel(0);
	comboProperty.SetCurSel(0);
	comboFeature.SetCurSel(0);
	comboLocation.SetCurSel(0);
	showOnlyFavorites = false;
	checkShowFavorites.SetCheck(false);
	editSearch.SetWindowText(_T(""));

}

void CMFCAppUniversityInfoExtractorDlg::OnBnClickedCheckShowFavorites()
{
	// 切换收藏筛选状态
    showOnlyFavorites = checkShowFavorites.GetCheck();
    ApplyFilters();
    UpdateListDisplay();
}

void CMFCAppUniversityInfoExtractorDlg::OnListClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItem = (LPNMITEMACTIVATE)pNMHDR;
    if (pNMItem->iItem >= 0 && pNMItem->iSubItem == 0) {
        // 收藏列点击
        CString name = listCtrl.GetItemText(pNMItem->iItem, 1);
        CT2A nameA(name);
        ToggleFavorite(nameA.m_psz);
        UpdateListDisplay();
    }
    *pResult = 0;
}

void CMFCAppUniversityInfoExtractorDlg::ToggleFavorite(const std::string& name)
{
    if (favoriteUniversities.count(name))
        favoriteUniversities.erase(name);
    else
        favoriteUniversities.insert(name);
    SaveFavorites();
}

void CMFCAppUniversityInfoExtractorDlg::LoadFavorites()
{
    favoriteUniversities.clear();
    std::ifstream fin("favorites.txt");
    std::string line;
    while (getline(fin, line)) {
        if (!line.empty()) favoriteUniversities.insert(line);
    }
}

void CMFCAppUniversityInfoExtractorDlg::SaveFavorites()
{
    std::ofstream fout("favorites.txt");
    for (const auto& name : favoriteUniversities) {
        fout << name << std::endl;
    }
}

void CMFCAppUniversityInfoExtractorDlg::OnStnClickedLabelSort() {}