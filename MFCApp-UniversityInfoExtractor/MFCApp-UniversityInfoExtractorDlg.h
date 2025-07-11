// MFCAppUniversityInfoExtractorDlg.h: 头文件
//

#pragma once

#include <vector>
#include <set>
#include "University.h"
#include "DataStorage.h"
#include <afxcmn.h>

// CMFCAppUniversityInfoExtractorDlg 对话框
class CMFCAppUniversityInfoExtractorDlg : public CDialogEx
{
// 构造
public:
	CMFCAppUniversityInfoExtractorDlg(CWnd* pParent = nullptr);	

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPUNIVERSITYINFOEXTRACTOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	


// 实现
protected:
	HICON m_hIcon;			// 主窗口图标

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl listCtrl;		 // 表格界面，用于在界面上以列表形式显示大学信息
	CEdit editSearch;			// 搜索框，用于输入搜索关键词
	CComboBox comboType;		 // 高校类型下拉框
	CComboBox comboProperty;	 // 性质下拉框
	CComboBox comboFeature;		 // 特色下拉框
	CComboBox comboSort;		 // 排序方式下拉框
	CComboBox comboLocation;	 // 所在地下拉框
	int sortOrder = 0;		// 0: 默认, 1: 名称升序, 2: 名称降序
	std::vector<University> allUniversities;			// 所有高校信息的完整列表
	std::vector<University> filteredUniversities;		// 筛选后的高校信息列表
	CStatic labelHint;		// 用于显示提示信息
	CProgressCtrl progressBar; // 进度条控件
	CStatic progressText;      // 进度文本

	std::set<std::string> favoriteUniversities; // 收藏高校名称集合
	BOOL showOnlyFavorites = FALSE; // 是否只显示收藏
	CButton checkShowFavorites; // 复选框控件

	void LoadUniversityData();	// 加载高校数据
	void UpdateListDisplay();	// 更新列表显示
	void ApplyFilters();		// 应用筛选条件
	void LoadFavorites(); // 加载收藏
	void SaveFavorites(); // 保存收藏
	void ToggleFavorite(const std::string& name); // 切换收藏

public:
	afx_msg void OnBnClickedButtonRefresh();	//消息处理函数
	afx_msg void OnEnChangeEditSearch();
	afx_msg void OnCbnSelchangeComboType();
	afx_msg void OnCbnSelchangeComboProperty();
	afx_msg void OnCbnSelchangeComboFeature();
	afx_msg void OnCbnSelchangeComboSort();
	afx_msg void OnCbnSelchangeComboLocation();	// 所在地下拉框选择变化
	afx_msg void OnBnClickedButtonResetFilters();// 清空筛选按钮
	afx_msg void OnBnClickedCheckShowFavorites(); // 收藏复选框点击

protected:
	afx_msg LRESULT OnRefreshDone(WPARAM, LPARAM);
	afx_msg void OnListRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListClick(NMHDR* pNMHDR, LRESULT* pResult); // 列表单击

public:
	static UINT RefreshThreadProc(LPVOID pParam);
	afx_msg void OnStnClickedLabelSort();
};
