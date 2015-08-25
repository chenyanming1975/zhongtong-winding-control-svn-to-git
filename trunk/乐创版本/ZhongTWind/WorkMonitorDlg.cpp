// WorkMonitorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ZhongTWind.h"
#include "WorkMonitorDlg.h"
#include "Mpc2810.h"
#include <math.h>
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CZhongTWindApp theApp;

#define  MPC2810_MAX_SPEED        2000000
#define  MPC2810_MIN_INITIAL_SPEED    100



#define WIND_METHOD_CIRCLE     1
#define WIND_METHOD_ADDSAND    2
#define WIND_METHOD_CROSS      3



#define BUTTON_ADDSAND_BIT          (1<<5)
#define BUTTON_CIRCLEWIND_BIT       (1<<6)
#define BUTTON_CLEARPOSZERO_BIT     (1<<7)
#define BUTTON_AUTO_BIT             (1<<8)
#define BUTTON_MANUAL_BIT           (1<<9)
#define BUTTON_CARSPEEDDOWN_BIT    (1<<10)
#define BUTTON_CARSPEEDUP_BIT      (1<<11) 
#define BUTTON_CARNEGDIR_BIT       (1<<12)
#define BUTTON_CARPOSDIR_BIT       (1<<13) 
#define BUTTON_MAINNEGDIR_BIT      (1<<14)
#define BUTTON_MAINPOSDIR_BIT      (1<<15) 

#define	 IO_FOR_MAINAXE_POSITIVE_DIR   4
#define	 IO_FOR_MAINAXE_NEGETIVE_DIR   3


	

#define  MAINAXE_FREQ_INVERT_N_CLOSE()  outport_bit(1,3,1)
#define  MAINAXE_FREQ_INVERT_N_OPEN()   outport_bit(1,3,0)

#define  MAINAXE_FREQ_INVERT_P_CLOSE()  outport_bit(1,4,1)
#define  MAINAXE_FREQ_INVERT_P_OPEN()   outport_bit(1,4,0)

#define  PI_VALUE        3.14159265359



/////////////////////////////////////////////////////////////////////////////
// CWorkMonitorDlg dialog


CWorkMonitorDlg::CWorkMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWorkMonitorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWorkMonitorDlg)
	m_WorkingOutHint = _T("");
	m_WorkingProgress1 = _T("");
	m_WorkingProgress2 = _T("");
	m_WorkingProgress3 = _T("");
	m_MainAxeSpeed = 0.0;
	m_MainAxePos = 0.0;
	m_MainAxeDir = _T("");
	m_CarPos = 0.0;
	m_CarSpeed = 0.0;
	m_CarDir = _T("");
	m_AlreadyWindNum = 0;
	m_PipeDiam = 0.0;
	m_BandageWidth = 0.0;
	m_StartPos = 0.0;
	m_EndPos = 0.0;
	m_StartStopAngle = 0.0;
	m_StopStopAngle = 0.0;
	m_WindNum = 0;
	m_CrossAngle = 0.0;
	m_WindStartLayer = 0;
	m_WindMethod = _T("");
	//}}AFX_DATA_INIT
}


void CWorkMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWorkMonitorDlg)
	DDX_Control(pDX, IDB_STARTWORK, m_BtnStartWork);
	DDX_Control(pDX, IDB_STOP_EMERG, m_Btn_StopCarEmerg);
	DDX_Control(pDX, IDB_STOP, m_BtnStopCar);
	DDX_Control(pDX, IDB_MEASURESTART, m_BtnMeasureStart);
	DDX_Control(pDX, IDB_INITIAL, m_ButtonInitial);
	DDX_Text(pDX, IDE_WORKING_OUTPUT, m_WorkingOutHint);
	DDX_Text(pDX, IDE_WORKING_PROGRESS_1, m_WorkingProgress1);
	DDX_Text(pDX, IDE_WORKING_PROGRESS_2, m_WorkingProgress2);
	DDX_Text(pDX, IDE_WORKING_PROGRESS_3, m_WorkingProgress3);
	DDX_Text(pDX, IDE_MAINAXE_SPEED, m_MainAxeSpeed);
	DDX_Text(pDX, IDE_MAINAXE_POS, m_MainAxePos);
	DDX_Text(pDX, IDE_MAINAXE_DIR, m_MainAxeDir);
	DDX_Text(pDX, IDE_CAR_POS, m_CarPos);
	DDX_Text(pDX, IDE_CAR_SPEED, m_CarSpeed);
	DDX_Text(pDX, IDE_CAR_DIR, m_CarDir);
	DDX_Text(pDX, IDE_ALREADY_WINDNUM, m_AlreadyWindNum);
	DDX_Text(pDX, IDE_PIPE_DIAM, m_PipeDiam);
	DDX_Text(pDX, IDE_SHAPIAN_WIDTH, m_BandageWidth);
	DDX_Text(pDX, IDE_START_POS, m_StartPos);
	DDX_Text(pDX, IDE_END_POS, m_EndPos);
	DDX_Text(pDX, IDE_START_STOPANGLE, m_StartStopAngle);
	DDX_Text(pDX, IDE_END_STOPANGLE, m_StopStopAngle);
	DDX_Text(pDX, IDE_WIND_NUM, m_WindNum);
	DDX_Text(pDX, IDE_CROSS_ANGLE, m_CrossAngle);
	DDX_Text(pDX, IDE_PUCENG_START, m_WindStartLayer);
	DDX_Text(pDX, IDE_WINDMETHOD, m_WindMethod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWorkMonitorDlg, CDialog)
	//{{AFX_MSG_MAP(CWorkMonitorDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDB_INITIAL, OnInitial)
	ON_BN_CLICKED(IDB_MEASURESTART, OnMeasurestart)
	ON_MESSAGE(WM_MMTIMER_MEASURE_SPEED, OnMMTimerMeasureSpeed)
	ON_BN_CLICKED(IDB_SET_ZEROPOT, OnSetZeropot)
	ON_BN_CLICKED(IDB_STOP, OnStop)
	ON_BN_CLICKED(IDB_STOP_EMERG, OnStopEmerg)
	ON_MESSAGE(WM_THREAD_BUTTON_CHANGE,OnButtonChanged)
	ON_MESSAGE(WM_THREAD_POS_UPDATE,OnUpdateCarPos)
	ON_BN_CLICKED(IDB_STARTWORK, OnStartwork)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkMonitorDlg message handlers

BOOL CWorkMonitorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	ShowWindow(SW_MAXIMIZE);
    LoadTask(0);

	m_MainAxeDir     = "<<+>>";
	m_MainAxePos     = 0;
	m_MainAxeSpeed   = 0;
	
	m_CarDir         = "<<+>>";
    m_CarPos         = 0;
	m_AlreadyWindNum = 0;
	m_WindStartLayer  = 1;
    UpdateData(false);



	m_QueryThreadRunningFlag = 0;
	m_queryThread = AfxBeginThread(QueryPressButton, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
    m_QueryThreadRunningFlag = 1;
    int intCheck;	
	intCheck = check_sfr(1);
    ButtonProcess(intCheck);
    m_queryThread->ResumeThread();


    m_CurrTaskIndex= 0;
	m_iMeasureInterval = 100;
	m_iUpdateInterval  = 200;
    m_iUpdateCount = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWorkMonitorDlg::LoadTask(int TaskIndex)
{
     m_PipeDiam       = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_PipeDiam;
	 m_BandageWidth   = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_shaWidth;
	 m_StartPos       = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartPos;
	 m_EndPos         = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_EndPos;
     m_StartStopAngle = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartStopAngle;
	 m_StopStopAngle  = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_EndStopAngle;
	 m_WindNum        = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_LayerNum;
	 m_CrossAngle     = m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_crossAngle;
	 
	 if (m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_WindMethod==WIND_METHOD_CIRCLE)
	 {
		 m_WindMethod = L"环向";
	 }
	 else if (m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_WindMethod==WIND_METHOD_ADDSAND)
	 {
		 m_WindMethod = L"加砂";
	 }
	 else if (m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_WindMethod==WIND_METHOD_CROSS)
	 {
		 m_WindMethod = L"交叉";
	 }
	 UpdateData(false);
}



UINT QueryPressButton(LPVOID pParam)   /// 线程函数
{   
    CWorkMonitorDlg* workingDlg = (CWorkMonitorDlg*)pParam;
    int intCheck;
	int LastIntCheck;
	LastIntCheck = 0;
	intCheck = 0;
	int firstflag=0;
    double pos;
	CString  pos_str;
	while (workingDlg->m_QueryThreadRunningFlag==1)
    {
		intCheck = check_sfr(1);
		
		if (LastIntCheck!=intCheck)
		{
			if (firstflag!=0)
			{
				SendMessage(workingDlg->GetSafeHwnd(),WM_THREAD_BUTTON_CHANGE,LastIntCheck,intCheck);
			}		   
			firstflag = 1;
		}
		LastIntCheck = intCheck;
		get_abs_pos(1,&pos);
		SendMessage(workingDlg->GetSafeHwnd(),WM_THREAD_POS_UPDATE,(WPARAM)(&pos),(LPARAM)(&pos));
		Sleep(200);
    }
	//ExitThread(0);
	return 0;
}

void CWorkMonitorDlg::ButtonProcess(int buttonStatus)
{
	UINT AndTemp1; 
	UINT AndTemp2; 
	if (buttonStatus&BUTTON_CLEARPOSZERO_BIT==0)  //清零按钮
	{
		MessageBox(L"标定零点");
		m_CarPos = 0;
		m_WorkingOutHint = L"标定零点成功!!!";
		set_abs_pos(1,0); 
		//get_encoder(2,&m_iniCarEncoderVal);
	}
	
	
	AndTemp1     = buttonStatus&BUTTON_CARNEGDIR_BIT;
	AndTemp2     = buttonStatus&BUTTON_CARPOSDIR_BIT;
	
	if ((AndTemp1)&&(AndTemp2))   //3档中间
	{
        ;
	}
	else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
	{
		m_CarDir         = ">>>>>>>";
	}
	else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
	{
		m_CarDir         = "<<<<<<<";
	} 
	else
	{
		MessageBox(_T("小车方向按钮出错!"));
	}




	AndTemp1     = buttonStatus&BUTTON_MAINNEGDIR_BIT;
	AndTemp2     = buttonStatus&BUTTON_MAINPOSDIR_BIT;

	if ((AndTemp1)&&(AndTemp2))   //3档中间
	{
		MAINAXE_FREQ_INVERT_N_CLOSE();
		MAINAXE_FREQ_INVERT_P_CLOSE();
	}
	else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
	{
		m_MainAxeDir         = ">>>>>>>";
		MAINAXE_FREQ_INVERT_N_CLOSE();
		MAINAXE_FREQ_INVERT_P_OPEN();
	}
	else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
	{
		m_MainAxeDir         = "<<<<<<<";
		MAINAXE_FREQ_INVERT_P_CLOSE();
		MAINAXE_FREQ_INVERT_N_OPEN();
	} 
	else
	{
		MessageBox(_T("主轴方向按钮出错!"));
	}



	
	AndTemp1     = buttonStatus&BUTTON_ADDSAND_BIT;
	AndTemp2     = buttonStatus&BUTTON_CIRCLEWIND_BIT;
	
	if ((AndTemp1)&&(AndTemp2))   //3档中间
	{
		;
	}
	else if((AndTemp1)&&(AndTemp2==0))//
	{
		;
	}
	else if ((AndTemp1==0)&&(AndTemp2))//
	{
		;
	} 
	else
	{
		MessageBox(_T("加沙环向按钮出错!"));
	}
	


	
	AndTemp1     = buttonStatus&BUTTON_AUTO_BIT;
	AndTemp2     = buttonStatus&BUTTON_MANUAL_BIT;
	
	if ((AndTemp1)&&(AndTemp2))   //3档中间
	{
		;
	}
	else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
	{
		;
	}
	else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
	{
		;
	} 
	else
	{
		MessageBox(_T("自动手动按钮出错!"));
	}
	
	
	 UpdateData(false);
}

LRESULT CWorkMonitorDlg::OnButtonChanged(WPARAM wparam, LPARAM lparam)
{
	UINT lastButton;
	UINT thisButton;
    UINT changed;
    UINT AndTemp1, AndTemp2;
    UINT AndTempLast1, AndTempLast2;
	lastButton = ((UINT)wparam)&0xFFFFFFFF;
    thisButton = ((UINT)lparam)&0xFFFFFFFF;
    
    changed = lastButton^thisButton;

    if (changed&BUTTON_CLEARPOSZERO_BIT)
    {
		AndTemp1 = thisButton&BUTTON_CLEARPOSZERO_BIT;
		if (AndTemp1) //按钮是按下后弹起
		{
			m_CarPos = 0;
			m_WorkingOutHint = L"标定零点成功";
		    set_abs_pos(1,0); 
			
			//get_encoder(2,&m_iniCarEncoderVal);
			//CString aa;
			//aa.Format(L"%.2f",(double)m_iniCarEncoderVal);
		    //MessageBox(aa);
		}
		else
		{

		}
    }
////////////////////////////////////////////小车加减速3档 按钮///////////////////////////////////////////////
	if ((changed&BUTTON_CARSPEEDUP_BIT)||(changed&BUTTON_CARSPEEDDOWN_BIT))
    {
		AndTemp1     = thisButton&BUTTON_CARSPEEDUP_BIT;
        AndTemp2     = thisButton&BUTTON_CARSPEEDDOWN_BIT;
		AndTempLast1 = lastButton&BUTTON_CARSPEEDUP_BIT;
        AndTempLast2 = lastButton&BUTTON_CARSPEEDDOWN_BIT;
		UINT andtemp11,andtemp12;
		andtemp11     = thisButton&BUTTON_CARNEGDIR_BIT;
		andtemp12     = thisButton&BUTTON_CARPOSDIR_BIT;
		if ((AndTemp1)&&(AndTemp2))   //3档中间
		{
			KillTimer(7);			  
		}
		else if((AndTemp1)&&(AndTemp2==0))//小车加速
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				SetTimer(7,500,0);
				m_flagTimerSpeed = 1;
                int checkstatus = check_status(1);  //读取指定轴的状态
                checkstatus = checkstatus&0x0001;    //LSB为1 停止
                if (checkstatus!=0)
                {				
					
					set_profile(1,100,theApp.GlobalParam.m_fAcceleration,theApp.GlobalParam.m_fAccelerationFactor); //m_fAcceleration = 8000
					m_CarCurSpeed = theApp.GlobalParam.m_fAcceleration;
					if (andtemp12==0)
					{
						fast_vmove(1,1);
					}
					else
					{
                        fast_vmove(1,-1);
					}
                }
				else
				{
					m_CarCurSpeed += theApp.GlobalParam.m_fAcceleration;
					if (m_CarCurSpeed>160000)
					{
						m_CarCurSpeed = 160000;
					}
					else
					{
                       change_speed(1,m_CarCurSpeed);
					}
					
				}
            }
		}
		else if ((AndTemp1==0)&&(AndTemp2))//小车小车减速
		{
			if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				SetTimer(7,500,0);
				m_flagTimerSpeed = 2;
                int checkstatus = check_status(1);
                checkstatus = checkstatus&0x0001;
                if (checkstatus!=0)
                {					
                    ;
                }
				else
				{
					m_CarCurSpeed -= theApp.GlobalParam.m_fAcceleration;
					if (m_CarCurSpeed<=0)
					{
						m_CarCurSpeed = 0;
						sudden_stop(1);
					}
					else
					{
                      change_speed(1,m_CarCurSpeed);
					}
					
				}
            }
		} 
		else
		{
			MessageBox(_T("减速按钮出错!"));
		}
    }
/////////////////////////////////////////小车加减速3档 按钮/////////////////////////////////////////////


////////////////////////////////////////////小车正反向3档 按钮///////////////////////////////////////////////
	if ((changed&BUTTON_CARNEGDIR_BIT)||(changed&BUTTON_CARPOSDIR_BIT))
    {
		AndTemp1     = thisButton&BUTTON_CARNEGDIR_BIT;
        AndTemp2     = thisButton&BUTTON_CARPOSDIR_BIT;
		AndTempLast1 = lastButton&BUTTON_CARNEGDIR_BIT;
        AndTempLast2 = lastButton&BUTTON_CARPOSDIR_BIT;
		if ((AndTemp1)&&(AndTemp2))   //3档中间
		{
            if ((AndTempLast1)&&(AndTempLast2==0))//原先正向被按下
            {
				m_CarDir = "<<<+>>>";		
				set_profile(1,100,theApp.GlobalParam.m_fAcceleration/2,theApp.GlobalParam.m_fAccelerationFactor);  //4000 500000
				decel_stop(1);
				m_CarCurSpeed =0;
				//MessageBox("小车正向归位");
            }
			else if ((AndTempLast1==0)&&(AndTempLast2))//原先反向被按下
			{
				m_CarDir = "<<<+>>>";
				set_profile(1,100,theApp.GlobalParam.m_fAcceleration/2,theApp.GlobalParam.m_fAccelerationFactor); 
				decel_stop(1);
				m_CarCurSpeed =0;
				//MessageBox("小车反向归位");
			}
		}
		else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				set_dir(1,1);//MessageBox("小车正向");
				m_CarDir = ">>>>>>>";
            }
		}
		else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				set_dir(1,-1);//MessageBox("小车反向");
				m_CarDir = "<<<<<<<";
            }
		} 
		else
		{
			MessageBox(_T("小车方向按钮出错!"));
		}
    }
/////////////////////////////////////////小车正反向3档 按钮/////////////////////////////////////////////





////////////////////////////////////////////主轴正反向3档 按钮//////////////////////////////////////////
	if ((changed&BUTTON_MAINNEGDIR_BIT)||(changed&BUTTON_MAINPOSDIR_BIT))
    {
		AndTemp1     = thisButton&BUTTON_MAINNEGDIR_BIT;
        AndTemp2     = thisButton&BUTTON_MAINPOSDIR_BIT;
		AndTempLast1 = lastButton&BUTTON_MAINNEGDIR_BIT;
        AndTempLast2 = lastButton&BUTTON_MAINPOSDIR_BIT;

		if ((AndTemp1)&&(AndTemp2))   //3档中间
		{

            if ((AndTempLast1)&&(AndTempLast2==0))//原先正向被按下
            {
				MAINAXE_FREQ_INVERT_N_CLOSE();
                MAINAXE_FREQ_INVERT_P_CLOSE();
				m_MainAxeDir = "<<<+>>>";
				//MessageBox("主轴正向归位");
            }
			else if ((AndTempLast1==0)&&(AndTempLast2))//原先反向被按下
			{
				MAINAXE_FREQ_INVERT_N_CLOSE();
                MAINAXE_FREQ_INVERT_P_CLOSE();
				m_MainAxeDir = "<<<+>>>";
				//MessageBox("主轴反向归位");
			}
		}

		else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				MAINAXE_FREQ_INVERT_N_CLOSE();
                MAINAXE_FREQ_INVERT_P_OPEN();
				m_MainAxeDir = ">>>>>>>";
				//MessageBox("主轴正向");
            }
			
			else if ((AndTempLast1==0)&&(AndTempLast2))//原先反向被按下
			{
				MAINAXE_FREQ_INVERT_N_CLOSE();
                MAINAXE_FREQ_INVERT_P_OPEN();
				//MessageBox("16");
				m_MainAxeDir = ">>>>>>>";
			}
		}
		else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				MAINAXE_FREQ_INVERT_P_CLOSE();
				MAINAXE_FREQ_INVERT_N_OPEN();
                
				//MessageBox("主轴反向");
				//outport_byte(1,16);
				m_MainAxeDir = "<<<<<<<";
            }
		} 
		else
		{
			MessageBox(_T("主轴方向按钮出错!"));
		}
    }
/////////////////////////////////////////主轴正反向3档 按钮/////////////////////////////////////////////
////////////////////////////////////////////手动自动3档 按钮////////////////////////////////////////////
	if ((changed&BUTTON_AUTO_BIT)||(changed&BUTTON_MANUAL_BIT))
    {
		AndTemp1     = thisButton&BUTTON_AUTO_BIT;
        AndTemp2     = thisButton&BUTTON_MANUAL_BIT;
		AndTempLast1 = lastButton&BUTTON_AUTO_BIT;
        AndTempLast2 = lastButton&BUTTON_MANUAL_BIT;

		if ((AndTemp1)&&(AndTemp2))   //3档中间
		{

            if ((AndTempLast1)&&(AndTempLast2==0))//原先正向被按下
            {
				MessageBox(L"自动归位");
            }
			else if ((AndTempLast1==0)&&(AndTempLast2))//原先反向被按下
			{
				MessageBox(L"手动归位");
			}
		}
		else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
		        ;
				MessageBox(L"自动");
            }
		}
		else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				MessageBox(L"手动");
            }
		} 
		else
		{
			MessageBox(_T("自动手动按钮出错!"));
		}
    }
/////////////////////////////////////////手动自动3档 按钮/////////////////////////////////////////////
////////////////////////////////////////////环向加沙按钮//////////////////////////////////////////////
	if ((changed&BUTTON_ADDSAND_BIT)||(changed&BUTTON_CIRCLEWIND_BIT))
    {
		AndTemp1     = thisButton&BUTTON_ADDSAND_BIT;
        AndTemp2     = thisButton&BUTTON_CIRCLEWIND_BIT;
		AndTempLast1 = lastButton&BUTTON_ADDSAND_BIT;
        AndTempLast2 = lastButton&BUTTON_CIRCLEWIND_BIT;
		
		if ((AndTemp1)&&(AndTemp2))   //3档中间
		{
			
            if ((AndTempLast1)&&(AndTempLast2==0))//原先正向被按下
            {
				MessageBox(L"加沙归位");
            }
			else if ((AndTempLast1==0)&&(AndTempLast2))//原先反向被按下
			{
				MessageBox(L"环向归位");
			}
		}
		else if((AndTemp1)&&(AndTemp2==0))//小车正向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				;
				MessageBox(L"加沙");
            }
		}
		else if ((AndTemp1==0)&&(AndTemp2))//小车反向被按下
		{
            if ((AndTempLast1)&&(AndTempLast2))//原先在中间
            {
				MessageBox(L"环向");
            }
		} 
		else
		{
			MessageBox(_T("环向加沙按钮出错!"));
		}
    }
///////////////////////////////////////////环向加沙按钮///////////////////////////////////////////////
    UpdateData(false);
    return 0;
}

void CWorkMonitorDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	int checkstatus;
	switch (nIDEvent)
    {
    case 7:
		if (m_flagTimerSpeed == 1)
		{
			m_CarCurSpeed += theApp.GlobalParam.m_fAcceleration/2;
			if (m_CarCurSpeed>160000)
			{
				m_CarCurSpeed = 160000;
			}
			else
			{
				change_speed(1,m_CarCurSpeed); 
			}
		}
		else
		{
			checkstatus = check_status(1);
			checkstatus = checkstatus&0x00001;
			if (checkstatus!=0)
			{					
				;
			}
			else
			{
				m_CarCurSpeed -= theApp.GlobalParam.m_fAcceleration/2;
				if (m_CarCurSpeed<=0)
				{
					m_CarCurSpeed = 0;
					sudden_stop(1);
				}
				else
				{
					change_speed(1,m_CarCurSpeed);
				}
			}		
		}
		break;
	case 8:		
		break;
    default:
		//	MessageBox("default: KillTimer");
		//KillTimer(nIDEvent);
		break;
    }	


	CDialog::OnTimer(nIDEvent);
}

void CWorkMonitorDlg::OnDestroy() 
{
	CDialog::OnDestroy();


	KillTimer(7);
	if (m_wTimerID)
	{
		timeKillEvent(m_wTimerID);	  
		timeEndPeriod(wTimerRes);
		m_wTimerID = 0;
		MessageBox(L"Destroy OK!");
	}
	sudden_stop(2);

	// TODO: Add your message handler code here
	
}

void CWorkMonitorDlg::OnInitial() 
{
	m_ButtonInitial.SetText(_T("初始化OK!"));
	m_ButtonInitial.EnableWindow(false);
	set_maxspeed(1,MPC2810_MAX_SPEED);
//	double distancev = (20/424.115001)*11*10000;

	int status = check_status(1);
	set_encoder_mode(1,1,1,0);//set_encoder_mode(1,1,1,0);
    set_encoder_mode(2,1,1,0);
	
  //  get_encoder(2,&m_iniCarEncoderVal);
	set_maxspeed(2,1000000);
//	set_conspeed(2,1000000);
//	con_vmove(2,1);
}

void CWorkMonitorDlg::OnMeasurestart() 
{
	// TODO: Add your control notification handler code here
	if (CreateTimer()==true)
	{
		MessageBox(L"MMTimer Set OK!");
		m_iLastEncoderValueMainAxe = 0;
		m_iThisEncoderValueMainAxe = 0;
		m_iUpdateCount = 0;
		
		m_BtnMeasureStart.EnableWindow(false);
	}
}





void  PASCAL MeasureMainAxeSpeedWork(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	CWorkMonitorDlg* localThisdlg = (CWorkMonitorDlg *)dwUser;

	localThisdlg->m_iUpdateCount++;
    localThisdlg->m_iLastEncoderValueMainAxe = localThisdlg->m_iThisEncoderValueMainAxe;
 // localThisdlg->m_2LastEncoderValue = localThisdlg->m_2ThisEncoderValue;
 //	get_encoder(2,&(localThisdlg->m_2ThisEncoderValue));
	get_encoder(1,&(localThisdlg->m_iThisEncoderValueMainAxe)); 

	localThisdlg->m_MainAxeDeltaEncoderCount[localThisdlg->m_iUpdateCount-1] = 
		localThisdlg->m_iThisEncoderValueMainAxe - localThisdlg->m_iLastEncoderValueMainAxe;
	
//	tempDelta = (double)(localThisdlg->m_2ThisEncoderValue - localThisdlg->m_2LastEncoderValue);
//    localThisdlg->m_TickEncoderDelta = localThisdlg->myAbs(tempDelta);
	
	
	
	if (localThisdlg->m_iUpdateCount>=(localThisdlg->m_iUpdateInterval/localThisdlg->m_iMeasureInterval))
	{
		PostMessage(localThisdlg->GetSafeHwnd(),WM_MMTIMER_MEASURE_SPEED,0,0);
		localThisdlg->m_iUpdateCount = 0;
	}
	
}










bool CWorkMonitorDlg::CreateTimer()
{
   
	TIMECAPS tc;
	
	if (timeGetDevCaps(&tc,sizeof(TIMECAPS))!=TIMERR_NOERROR)
	{
		return FALSE;
	}
	
	wTimerRes = min(max(tc.wPeriodMin,1),tc.wPeriodMax);
	
	timeBeginPeriod(wTimerRes);
	m_wTimerID = timeSetEvent(m_iMeasureInterval,wTimerRes,(LPTIMECALLBACK)MeasureMainAxeSpeedWork,(DWORD)this,TIME_PERIODIC);
	
	if (m_wTimerID==0)
	{
		return FALSE;
	}	
    return TRUE;

}



LRESULT CWorkMonitorDlg::OnMMTimerMeasureSpeed(WPARAM wparam, LPARAM lparam)
{
	
	CString showStr;
//    this->m_LastEncoder_Show = m_iLastEncoderValue;
//    this->m_ThisEncoder_Show = m_iThisEncoderValue;
	double tempdouble;
	double MainAxeEncoder_resolution;
	double MainAxeRatio;
	double RealV;
//	m_test1 = m_iLastEncoderValue;
//  m_test2 = m_iThisEncoderValue;
    MainAxeEncoder_resolution = (double)(theApp.GlobalParam.m_iMainAxePulses);
	if (m_iThisEncoderValueMainAxe<0)
	{
		;
	}
    int i,num;
	long MeanDelta;
	MeanDelta = 0;
	
	num = (this->m_iUpdateInterval/this->m_iMeasureInterval);
    for (i =0 ;i<num;i++)
    {
		MeanDelta += m_MainAxeDeltaEncoderCount[i];
    }
	
	
	
	// 编码器差值除以时间，时间单位是毫秒，所以乘以1000，编码器分辨率是2000，所以除以2000 得到转速
	tempdouble = ((double)(m_iThisEncoderValueMainAxe - m_iLastEncoderValueMainAxe))*1000/
		((double)m_iMeasureInterval*MainAxeEncoder_resolution);
	showStr.Format(L"%.5f",tempdouble);
	
	MainAxeRatio = this->m_lpWorkPlanDlg->m_WindTask[m_CurrTaskIndex].m_MainAxeSpeedFactor;
    this->m_MainAxeSpeed = tempdouble*60/MainAxeRatio; //得到每分钟多少转
    
	RealV = (double)(m_iThisEncoderValueMainAxe - m_iLastEncoderValueMainAxe)*(PI_VALUE)*( this->m_lpWorkPlanDlg->m_WindTask[m_CurrTaskIndex].m_PipeDiam)*1000/
		(((double)m_iMeasureInterval)*MainAxeEncoder_resolution*MainAxeRatio);
    RealV = ((double)(MeanDelta))*(PI_VALUE)*(this->m_lpWorkPlanDlg->m_WindTask[m_CurrTaskIndex].m_PipeDiam)*1000/
		(((double)m_iMeasureInterval)*MainAxeEncoder_resolution*MainAxeRatio*(double)num);
    
	//RealV = (double)(m_iThisEncoderValue - m_iLastEncoderValue)*(PI_VALUE)*(m_lpOpDlg->m_WindTask[0].m_PipeDiam)*1000/
	//	              ((m_TickEncoderDelta/1000)*MainAxeEncoder_resolution*MainAxeRatio);
	
	//  m_MainAxeRealV = m_MainAxeRealV/1000;
    
	if (RealV<0)
	{
		m_MainAxeRealV=RealV*(-1);
	} 
    else
	{
        m_MainAxeRealV = RealV;
	}
	
    UpdateData(false);
	
	return 0;
}



BOOL CWorkMonitorDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DestroyWindow();
}

void CWorkMonitorDlg::OnSetZeropot() 
{
	// TODO: Add your control notification handler code here
	
}

void CWorkMonitorDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	decel_stop(1);	
	double pos;
	get_abs_pos(1,&pos);
    this->m_WindProcessRunningFlag = 0;
    sudden_stop(1);
}

void CWorkMonitorDlg::OnStopEmerg() 
{
	// TODO: Add your control notification handler code here
	double pos;
 //   long encoderCar;
	get_abs_pos(1,&pos);
  //  get_encoder(2,&encoderCar);
	
	set_profile(1,100,20000,20000);
    get_abs_pos(1,&pos);
    fast_pmove(1,0-pos);
}

void CWorkMonitorDlg::OnStartwork() 
{
	// TODO: Add your control notification handler code here
	if (myAbs(m_CarPos)<0.0001)
    {
		m_BtnStartWork.EnableWindow(false);
		m_WindProcThread = AfxBeginThread(WindingProcessThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
		this->m_WindProcessRunningFlag = 1;
		m_WindProcThread->ResumeThread();
    }
    else
	{
		MessageBox(L"请先归零");
	}

}

double CWorkMonitorDlg::myAbs(double a)
{
	double ret;
	ret = a * a;
	ret = sqrt(ret);
    return ret;
}



LRESULT CWorkMonitorDlg::OnUpdateCarPos(WPARAM wparam, LPARAM lparam)
{
	
	double Car_RoundRatio;
	double Car_DistancePerRound;
	double Car_CyclePulse;
	double *pos;
	
	Car_RoundRatio = theApp.GlobalParam.m_fCarSpeedDownP;  //m_fCarSpeedDownP;
	Car_DistancePerRound = theApp.GlobalParam.m_fCarDistancePerRound;
	Car_CyclePulse = (double)(theApp.GlobalParam.m_iCarPulses);	
	
	pos = (double *)wparam; 
	
	m_CarPos = ((*pos)*Car_DistancePerRound)/(Car_RoundRatio*Car_CyclePulse);

	UpdateData(false);
    
	return 0;
}


UINT WindingProcessThread(LPVOID pParam)   /// 线程函数
{   
    CWorkMonitorDlg* workingDlg = (CWorkMonitorDlg*)pParam;
	CarControlParam CarParam,CarNewParam;
	
	double MainAxeSpeed;
	double buffLen;
	double TotalLen;
	double StopPos;
	double startpos;
	double qitingjiao;
    int TaskIndex;
	int runningflag;
	double EncoderResolution;
	
	int N;
    int i;
	
	TaskIndex = workingDlg->m_CurrTaskIndex;
    N = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_LayerNum;

	workingDlg->ComputeCarMoveParam(TaskIndex,&CarParam);
	
	EncoderResolution = theApp.GlobalParam.m_iMainAxePulses;
    qitingjiao = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartStopAngle;
	MainAxeSpeed = workingDlg->m_MainAxeRealV;
	
	buffLen = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_BuffLen;
	TotalLen = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_EndPos - workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartPos;
	StopPos = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_EndPos;
	startpos = workingDlg->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartPos;
    
	int ret ;
	
	runningflag = 1;
	ret = -1;
	for (i =TaskIndex;i< workingDlg->m_lpWorkPlanDlg->m_TaskNum;i++)
	{
		if (workingDlg->m_lpWorkPlanDlg->m_WindTask[i].m_WindMethod == WIND_METHOD_CIRCLE)
		{
			ret = workingDlg->CircleWindProcess(i,&(workingDlg->m_WindProcessRunningFlag));
		}
		else if (workingDlg->m_lpWorkPlanDlg->m_WindTask[i].m_WindMethod == WIND_METHOD_CROSS)
		{   //CrossWindProcess
			ret = workingDlg->CrossWindProcessq(i,&(workingDlg->m_WindProcessRunningFlag));
			//ret = workingDlg->CrossWindProcess(i,&(workingDlg->m_WindProcessRunningFlag));
		}
		
		if ( ret == 0)
		{
			workingDlg->m_BtnStartWork.EnableWindow(TRUE);
			return 0;
		}
		else if (ret == 1)
		{
			ret = -1;
		}
	}
	
    //workingDlg->CrossWindProcess(TaskIndex,&(workingDlg->m_WindProcessRunningFlag));
	workingDlg->m_BtnStartWork.EnableWindow(TRUE);
	return 0;
}


void CWorkMonitorDlg::ComputeCarMoveParam(int index, CarControlParam *lpparam)
{
   	double Car_a;
	double Car_ObjSpeed;
	double MainAxeSpeed;
	double Angle;
	double Car_RoundRatio;
	double Car_DistancePerRound;
    double Car_CyclePulse;
	double V0;
	double buffLen;
    int TaskIndex;
	index = index;
    MainAxeSpeed = this->m_MainAxeRealV;
	TaskIndex = index ;//this->m_CurTaskIndex;
	Angle = this->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_crossAngle;
	Angle = Angle*PI_VALUE/180;
	
	Car_RoundRatio = theApp.GlobalParam.m_fCarSpeedDownP;
	Car_DistancePerRound = theApp.GlobalParam.m_fCarDistancePerRound;
	Car_CyclePulse = (double)( theApp.GlobalParam.m_iCarPulses );
	
    Car_ObjSpeed = MainAxeSpeed/tan(Angle);
	
    V0 = 100*Car_DistancePerRound/(Car_RoundRatio*Car_CyclePulse);
	
	buffLen = this->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_BuffLen;
	
    Car_a = (Car_ObjSpeed*Car_ObjSpeed - V0*V0)/(2*buffLen);  //单位都是毫米
	Car_a = (Car_ObjSpeed*Car_ObjSpeed - 0*0)/(2*buffLen);  //单位都是毫米
    
	Car_a = (Car_a/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
	Car_ObjSpeed = (Car_ObjSpeed/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
	
	
    lpparam->m_a = Car_a;
    lpparam->m_ObjSpeed = Car_ObjSpeed; 


}

int CWorkMonitorDlg::CircleWindProcess(int index, int *runningflag)
{
   	int iWindLayerNum;
	CarControlParam CarParam,CarNewParam;
	double Car_a;
	double Car_ObjSpeed;
	double Car_CurPos;
	double MainAxeSpeed;
	double Car_RoundRatio;
	double Car_DistancePerRound;
	double Car_CyclePulse;
	double buffLen;
	double TotalLen;
	double StopPos;
	double startpos;
	int TaskIndex;
	//	double iniMainAxeEncoder;
	//	double CurMainAxeEncoder;
	double EncoderResolution;
	int Car_dir;
	int i;
    double mainAxeRatio;
	//	double Car_CurPosInmm;
    double distanceStartPos;
	
	iWindLayerNum = this->m_lpWorkPlanDlg->m_WindTask[index].m_LayerNum;////
	ComputeCarMoveParam(0,&CarParam);
	mainAxeRatio = this->m_lpWorkPlanDlg->m_WindTask[index].m_MainAxeSpeedFactor;
	EncoderResolution = theApp.GlobalParam.m_iMainAxePulses;

	//	qitingjiao = this->m_lpOpDlg->m_WindTask[TaskIndex].m_StartStopAngle;     // 起停角
	MainAxeSpeed = this->m_MainAxeRealV;  //主轴速度
	TaskIndex = index;   
	Car_RoundRatio = theApp.GlobalParam.m_fCarSpeedDownP;
	Car_DistancePerRound = theApp.GlobalParam.m_fCarDistancePerRound;
	Car_CyclePulse = (double)(theApp.GlobalParam.m_iCarPulses);	
	
	buffLen = this->m_lpWorkPlanDlg->m_WindTask[index].m_BuffLen;//缓冲长度
	TotalLen = this->m_lpWorkPlanDlg->m_WindTask[index].m_EndPos - 
		this->m_lpWorkPlanDlg->m_WindTask[index].m_StartPos;  //起点减去终点
	if (TotalLen>0)
	{
		Car_dir = 1;
	}
	else	
	{
		Car_dir = -1;
	}
	StopPos = this->m_lpWorkPlanDlg->m_WindTask[index].m_EndPos;   //终点
	startpos =this->m_lpWorkPlanDlg->m_WindTask[index].m_StartPos; //起点
	
	
	for (i = 0;i<iWindLayerNum;i++)
	{
		get_abs_pos(1,&Car_CurPos);
		distanceStartPos = (startpos/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
		fast_pmove(1,distanceStartPos - Car_CurPos);  //到起点
		
		while(check_done(1))
		{
			Sleep(100); 
		}
		
		this->ComputeCarMoveParam(0,&CarParam);
		Car_a = CarParam.m_a;
		Car_ObjSpeed = CarParam.m_ObjSpeed;
		
		set_profile(1,100,Car_ObjSpeed,Car_a);
        distanceStartPos = ((StopPos - startpos)/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
        
		fast_pmove(1,distanceStartPos - Car_CurPos);
		while((check_done(1))&&(*runningflag))
		{
			this->ComputeCarMoveParam(0,&CarNewParam);
			if (this->myAbs(CarNewParam.m_ObjSpeed - CarParam.m_ObjSpeed)>0.08*this->myAbs(CarParam.m_ObjSpeed)) //变化量大于05%就变速
			{
				CarParam.m_ObjSpeed = CarNewParam.m_ObjSpeed;
				CarParam.m_a = CarNewParam.m_a;
				set_profile(1,100,CarParam.m_ObjSpeed,Car_a);
				change_speed(1,CarParam.m_ObjSpeed);
			}
			Sleep(100);
		}
		if ((*runningflag)==0)
		{
			return 0;
		}
        sudden_stop(1);
		
	}
	
    return 1;



}

int CWorkMonitorDlg::CrossWindProcessq(int index, int *runningflag)
{

	int iWindLayerNum;
	CarControlParam CarParam,CarNewParam;
	double Car_a;
	double Car_ObjSpeed;
	double Car_CurPos;
	double MainAxeSpeed;
	double Car_RoundRatio;
	double Car_DistancePerRound;
	double Car_CyclePulse;
	double buffLen;
	double TotalLen;
	double StopPos;
	double startpos;
	double qitingjiao;
	int TaskIndex;
	long iniMainAxeEncoder;
	long CurMainAxeEncoder;
	double EncoderResolution;
	int Car_dir;
	int i;
    double distanceStartPos;
	long StartPosEncoder;
    long EndPosEncoder;
	long lastEncoderV;
    long midPosEncoder;
	double MainAxePos_Y;
    double MainAxeRatio;
    double deltaMainCircle;
    double MainAxeCircleLen;
    double adjustangle;
 
    CString debugOutstr;
    double temp_CurSpeed;

	TaskIndex = index;
	iWindLayerNum = this->m_lpWorkPlanDlg->m_WindTask[index].m_LayerNum;
	ComputeCarMoveParam(index,&CarParam);
	EncoderResolution = theApp.GlobalParam.m_iMainAxePulses;   //主轴编码器分辨率

	MainAxeRatio = this->m_lpWorkPlanDlg->m_WindTask[index].m_MainAxeSpeedFactor;
	qitingjiao = this->m_lpWorkPlanDlg->m_WindTask[TaskIndex].m_StartStopAngle;     // 起停角
	MainAxeSpeed = this->m_MainAxeRealV;  //主轴速度	   
    
	MainAxeCircleLen = this->m_lpWorkPlanDlg->m_WindTask[index].m_PipeDiam*PI_VALUE;
	Car_RoundRatio = theApp.GlobalParam.m_fCarSpeedDownP;
	Car_DistancePerRound = theApp.GlobalParam.m_fCarDistancePerRound;
	Car_CyclePulse = (double)(theApp.GlobalParam.m_iCarPulses);	
	   
	buffLen  = this->m_lpWorkPlanDlg->m_WindTask[index].m_BuffLen;//缓冲长度
	TotalLen = this->m_lpWorkPlanDlg->m_WindTask[index].m_EndPos -  
	                           this->m_lpWorkPlanDlg->m_WindTask[index].m_StartPos;  //起点减去终点
	if (TotalLen>0)
	{
	  Car_dir = 1;
	}
	else	
	{
	  Car_dir = -1;
	}
	StopPos = this->m_lpWorkPlanDlg->m_WindTask[index].m_EndPos;   //终点
	startpos = this->m_lpWorkPlanDlg->m_WindTask[index].m_StartPos; //起点
	 
	MainAxePos_Y = 0;
	
	this->ComputeCarMoveParam(TaskIndex,&CarParam);
	Car_a = CarParam.m_a;
	Car_ObjSpeed = CarParam.m_ObjSpeed;
	set_profile(1,20,Car_ObjSpeed,Car_a);
	set_maxspeed(1,Car_ObjSpeed);
/////////////////////////////////////////////////////////////////////
	/*
	CString astr;
	astr.Format("%.2f",Car_a);
	astr+="\r\n";
	SendMessage(WM_DEBUGOUTPUT,(WPARAM)(&astr),astr.GetLength());
	*/
/////////////////////////////////////////////////////////////////////////
    

	get_encoder(1,&StartPosEncoder);
	lastEncoderV = StartPosEncoder;
    for (i = 0;i<iWindLayerNum;i++)
	{
		/*
		get_abs_pos(1,&Car_CurPos);
		distanceStartPos = (startpos/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
		fast_pmove(1,distanceStartPos - Car_CurPos);  //到起点
        while(check_done(1))
		{
			Sleep(100); 
		}
		*/
		
		
        distanceStartPos = ((StopPos - startpos)/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
       // get_abs_pos(1,&Car_CurPos);
	   //double curTorch = Car_CurPos;
		fast_pmove(1,distanceStartPos );
		//fast_pmove(1,distanceStartPos - Car_CurPos);
	
		int monitorFlag = 0;
		while((check_done(1))&&(*runningflag))
		{
			this->ComputeCarMoveParam(TaskIndex,&CarNewParam);
			if (this->myAbs(CarNewParam.m_ObjSpeed - CarParam.m_ObjSpeed)>0.1*this->myAbs(CarParam.m_ObjSpeed)) //变化量大于10%就变速
			{
			    /*
				CarParam.m_ObjSpeed = CarNewParam.m_ObjSpeed;
				CarParam.m_a = CarNewParam.m_a;
				set_profile(1,20,CarParam.m_ObjSpeed,Car_a);
				change_speed(1,CarParam.m_ObjSpeed);
				*/
				;
			}
			/*
			temp_CurSpeed = get_rate(1);
            if ((monitorFlag==0)&&(temp_CurSpeed>=Car_ObjSpeed))
            {
                double curPos;
				get_abs_pos(1,&curPos);
				curPos = (curPos-curTorch)*Car_DistancePerRound/(Car_RoundRatio*Car_CyclePulse);
				debugOutstr.Format("%f",curPos);
                debugOutstr = "1: up speed at:" + debugOutstr + "\r\n";
				SendMessage(WM_DEBUGOUTPUT,(WPARAM)(&debugOutstr),debugOutstr.GetLength());
				monitorFlag = 1;
            }
            if ((monitorFlag==1)&&(temp_CurSpeed<Car_ObjSpeed))
            {
                double curPos;
				get_abs_pos(1,&curPos);
				curPos = (curPos-curTorch)*Car_DistancePerRound/(Car_RoundRatio*Car_CyclePulse);
				debugOutstr.Format("%f",curPos);
                debugOutstr = "2: down speed at:" + debugOutstr + "\r\n";
				SendMessage(WM_DEBUGOUTPUT,(WPARAM)(&debugOutstr),debugOutstr.GetLength());
				monitorFlag = 2;
            }
            */

			//Sleep(10);
            get_encoder(1,&midPosEncoder);
            deltaMainCircle = myAbs((double)(midPosEncoder - lastEncoderV))/(MainAxeRatio*(double)EncoderResolution);
            lastEncoderV = midPosEncoder;
            deltaMainCircle = deltaMainCircle * MainAxeCircleLen;
            MainAxePos_Y += deltaMainCircle;
		}
        if (*runningflag==0)
        {
			return 0;
        }




        //sudden_stop(1);
		int angleflag = 1;
		get_encoder(1,&iniMainAxeEncoder); 
		double ObjdeltaEncoder  =  (qitingjiao)*EncoderResolution*(this->m_lpWorkPlanDlg->m_WindTask[index].m_MainAxeSpeedFactor)/(360);
		 ObjdeltaEncoder  =  (360)*EncoderResolution*(this->m_lpWorkPlanDlg->m_WindTask[index].m_MainAxeSpeedFactor)/(360);
		while((angleflag)&&((*runningflag))) //等待起停角
		{
			//Sleep(1);
			get_encoder(1,&CurMainAxeEncoder); 
			midPosEncoder = CurMainAxeEncoder;
            deltaMainCircle = myAbs((double)(midPosEncoder - lastEncoderV))/(MainAxeRatio*(double)EncoderResolution);
            lastEncoderV = midPosEncoder;
            deltaMainCircle = deltaMainCircle * MainAxeCircleLen;
            MainAxePos_Y += deltaMainCircle;
			if (this->myAbs(CurMainAxeEncoder-iniMainAxeEncoder)
				                               >= ObjdeltaEncoder)
			{
				angleflag = 0;
			}

		}
		if ((*runningflag)==0)
		{
			return 0;
		}

		set_profile(1,20,Car_ObjSpeed,Car_a);
        distanceStartPos = ((startpos - StopPos)/Car_DistancePerRound)*Car_RoundRatio*Car_CyclePulse;
        get_abs_pos(1,&Car_CurPos);

        fast_pmove(1,distanceStartPos);
        while((check_done(1))&&(*runningflag))
		{
			this->ComputeCarMoveParam(TaskIndex,&CarNewParam);
			if (this->myAbs(CarNewParam.m_ObjSpeed - CarParam.m_ObjSpeed)>0.1*this->myAbs(CarParam.m_ObjSpeed)) //变化量大于10%就变速
			{
				/*
				CarParam.m_ObjSpeed = CarNewParam.m_ObjSpeed;
				CarParam.m_a = CarNewParam.m_a;
				set_profile(1,0,CarParam.m_ObjSpeed,Car_a);
				change_speed(1,CarParam.m_ObjSpeed);
                */
            }
			//Sleep(10);
			get_encoder(1,&midPosEncoder);
            deltaMainCircle = myAbs((double)(midPosEncoder - lastEncoderV))/(MainAxeRatio*(double)EncoderResolution);
            lastEncoderV = midPosEncoder;
            deltaMainCircle = deltaMainCircle * MainAxeCircleLen;
            MainAxePos_Y += deltaMainCircle;
		}
		if (*runningflag==0)
        {
			return 0;
        }
		
       // sudden_stop(1);
		get_encoder(1,&midPosEncoder);
		deltaMainCircle = myAbs((double)(midPosEncoder - lastEncoderV))/(MainAxeRatio*(double)EncoderResolution);
		lastEncoderV = midPosEncoder;
		deltaMainCircle = deltaMainCircle * MainAxeCircleLen;
        MainAxePos_Y += deltaMainCircle;        
		adjustangle = AdjustAngle(MainAxePos_Y+MainAxeCircleLen*qitingjiao/360,MainAxeCircleLen,TaskIndex);

        double tempout = 360*MainAxePos_Y/MainAxeCircleLen;
 //       SendMessage(WM_DEBUGOUTPUTMM,(WPARAM)(&tempout),(LPARAM)(&adjustangle));

		angleflag = 1;
		get_encoder(1,&iniMainAxeEncoder); 

        tempout = qitingjiao+adjustangle;
        if (tempout>180+360)
        {
           tempout-=360;
        }
		ObjdeltaEncoder  = (tempout)*EncoderResolution*(this->m_lpWorkPlanDlg->m_WindTask[index].m_MainAxeSpeedFactor)/(360);
	//	ObjdeltaEncoder  = 360*EncoderResolution*(m_lpOpDlg->m_WindTask[index].m_mainAxeRatio)/(360);
		while((angleflag)&&((*runningflag))) //等待起停角
		{
		//	Sleep(1);
			get_encoder(1,&CurMainAxeEncoder); 
			if (this->myAbs(CurMainAxeEncoder-iniMainAxeEncoder) 
				>= ObjdeltaEncoder)
			{
				angleflag = 0;
			}		   
		}
		if ((*runningflag)==0)
		{
			return 0;
		}
		/*
        get_encoder(1,&EndPosEncoder);
        CString deltaAStr;
		CString outStr;
		outStr = "delta Angle : ";
        deltaAStr.Format("%f",(double)(EndPosEncoder - StartPosEncoder));
        outStr+=deltaAStr;
        outStr+="\r\n";
		SendMessage(WM_DEBUGOUTPUT,(WPARAM)(&outStr),outStr.GetLength());
		SendMessage(WM_WIND_NUM_UPDATE,i+1,i+1);
		*/
		MainAxePos_Y = 0;
		get_encoder(1,&StartPosEncoder);
    	lastEncoderV = StartPosEncoder;
	}

    return 1;



}

double CWorkMonitorDlg::AdjustAngle(double nowlength, double circlelen, int i)
{
	int N;
	double a;
    double angle;
    double width;
	double ret;
	
	angle = this->m_lpWorkPlanDlg->m_WindTask[i].m_crossAngle;
	angle = angle*PI_VALUE/180;
	
	width = this->m_lpWorkPlanDlg->m_WindTask[i].m_shaWidth;
    N = (int)(nowlength/circlelen);
	
	a = nowlength - ((double)N)*circlelen;
	
	if (a <= width*tan(angle))
	{
		ret = width*tan(angle) - a;
	}
	else	
	{
		ret = circlelen - a + width*tan(angle);
	}
    ret = ret/circlelen;
	ret = ret * 360;
    return ret ;
	

}
