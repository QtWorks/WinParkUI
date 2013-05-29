#include "dlgstaying.h"
#include "ui_dlgstaying.h"
#include "Common/logicinterface.h"

CDlgStaying::CDlgStaying(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgStaying)
{
    ui->setupUi(this);
    CCommonFunction::SetWindowIcon( this );
    bHistory = CCommonFunction::GetSettings( CommonDataType::CfgSystem )->value( "CommonCfg/HistoryDb", false ).toBool( );

    pSystem = CCommonFunction::GetSettings( CommonDataType::CfgSystem );

    SetChkClikedArray( false );

    GetData( );
    pFrmDisplayPic =  new CPrintYearlyReport( NULL, this );
    SetFrameVisble( false );

    pFrmDisplayPic->move( geometry( ).width( ) - pFrmDisplayPic->width( ),
                          geometry( ).height( ) - pFrmDisplayPic->height( ) );
}

void CDlgStaying::SetFrameVisble( bool bVisible )
{
    pFrmDisplayPic->setVisible(  bVisible );
}

void CDlgStaying::closeEvent(QCloseEvent *)
{
    pSystem->setValue( "Staying/ASC", ui->cbSort->currentIndex( ) );

    for ( int n = 0; n < 7; n++ ) {
        if ( bChkCliked[ n ] ) {
            if ( 2 == n || 2 == n ) {
                n = 0;
            }

            pSystem->setValue( "Staying/Column", n );
            break;
        }
    }
}

CDlgStaying::~CDlgStaying()
{
    delete pFrmDisplayPic;
    delete ui;
}

void CDlgStaying::GetMonthData( QString &strOrder )
{
    QString strSql = "SELECT d.cardno,d.cardselfno, b.username, b.userphone, c.carcp, a.inshebeiname, a.intime \
            FROM stoprd a, userinfo b, carinfo c, monthcard d \
            where a.stoprdid = ( select stoprdid from cardstoprdid c \
                                 where d.cardno = c.cardno and d.Inside = 1 ) and a.outtime is null \
                and d.cardno = b.cardindex and d.cardno = c.cardindex " + strOrder;

    QStringList lstRows;
    int nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstRows, bHistory );
    if ( 0 < nRows ) {
       FillTable( lstRows, ui->tableWidgetMonth, nRows );
    }
}

void CDlgStaying::GetTimeData( QString &strOrder )
{
    QStringList lstRows;
    QString strSql = "SELECT b.cardno,b.cardselfno, a.carcp, a.inshebeiname, a.intime \
            FROM stoprd a, tmpcard b \
            where a.stoprdid = ( select stoprdid from cardstoprdid c \
                                 where b.cardno = c.cardno and b.Inside = 1 ) and a.outtime is null" + strOrder;

    lstRows.clear( );
    int nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstRows, bHistory );
    if ( 0 < nRows ) {
      FillTable( lstRows, ui->tableWidgetTime, nRows );
    }
}

void CDlgStaying::GetData( )
{
    int nIndex = ui->tableWidgetMonth->columnCount( ) - 1;
    QHeaderView* pView = ui->tableWidgetMonth->horizontalHeader( );
    pView->resizeSection( nIndex, pView->sectionSize( nIndex ) * 2 );

    nIndex = ui->tableWidgetTime->columnCount( ) - 1;
    pView = ui->tableWidgetTime->horizontalHeader( );
    pView->resizeSection( nIndex, pView->sectionSize( nIndex ) * 2 );

    QString strOrder = " Order by cardno asc ";
    int nCb = pSystem->value( "Staying/ASC", 0 ).toInt( );
    int nChk = pSystem->value( "Staying/Column", 0 ).toInt( );

    ui->cbSort->setCurrentIndex( nCb );
    QRadioButton* pBtn = findChild< QRadioButton* >( QString( "chk%1" ).arg( nChk ) );
    if ( NULL != pBtn ) {
        pBtn->setChecked( true );
    }

    GetOrderByClause( strOrder, nChk, nCb );
    GetMonthData( strOrder );
    GetTimeData( strOrder );
}

void CDlgStaying::FillTable( QStringList &lstData, QTableWidget *pTable, int nRows )
{
    //int nIndex = pTable->columnCount( ) - 1;
    //QHeaderView* pView = pTable->horizontalHeader( );
    //pView->resizeSection( nIndex, pView->sectionSize( nIndex ) * 2 );

    CCommonFunction::FillTable( pTable, nRows, lstData );
}

void CDlgStaying::on_tableWidgetMonth_cellClicked(int row, int column)
{
    DisplayPic( ( QTableWidget* ) sender( ), row, column );
}

void CDlgStaying::on_tableWidgetTime_cellClicked(int row, int column)
{
    DisplayPic( ( QTableWidget* ) sender( ), row, column );
}

void CDlgStaying::DisplayPic( QTableWidget* pWidget, int nRow, int nCol )
{
    bool bZeroCol = ( 0 == nCol );
    bool bExist = false;

    if ( bZeroCol ) {
        QString strFile;
        QString strWhere = QString( " Where cardno ='%1' and intime = '%2' " ).arg(
                    pWidget->item( nRow, nCol )->text( ), pWidget->item( nRow, pWidget->columnCount( ) - 1 )->text( ) );
        CCommonFunction::GetPath( strFile, CommonDataType::PathSnapshot );
        strFile += "Staying.jpg";
        CLogicInterface::GetInterface( )->OperateBlob( strFile, false, CommonDataType::BlobVehicleIn1, strWhere, bHistory );

        bExist = QFile::exists( strFile );
        if ( bExist ) {
            pFrmDisplayPic->DisplayPicture( strFile );
            QFile::remove( strFile );
        }
    }

    bExist = bExist &&  bZeroCol;
    SetFrameVisble( bExist );
}

void CDlgStaying::on_tabWidget_currentChanged(int index)
{
    SetChkClikedArray( false );
    SetFrameVisble( false );

    bool bMonth = ( 0 == index );
    ui->chk2->setEnabled( bMonth );
    ui->chk3->setEnabled( bMonth );
}

void CDlgStaying::SetChkClikedArray( bool bInit )
{
    for ( int n = 0; n < 7; n++ ) {
        bChkCliked[ n ] = bInit;
    }
}

void CDlgStaying::SortData( int nChk, int nCb, bool bCb )
{
    if ( !bCb && GetClicked( nChk ) ) {
        return;
    }

    QString strOrder;
    GetOrderByClause( strOrder, nChk, nCb );

    GetMonthData( strOrder );

    if ( 2 != nChk && 3 != nChk ) {
        GetTimeData( strOrder );
    }

    return;

    switch ( ui->tabWidget->currentIndex( ) ) {
    case 0 :
        GetMonthData( strOrder );
        break;

    case 1 :
        GetTimeData( strOrder );
        break;
    }
}

int CDlgStaying::GetChkIndex( )
{
    int nIndex = 0;

    if ( ui->chk0->isChecked( ) ) {
       nIndex = 0;
    } else if ( ui->chk1->isChecked( ) ) {
       nIndex = 1;
    } else if ( ui->chk2->isChecked( ) ) {
       nIndex = 2;
    } else if ( ui->chk3->isChecked( ) ) {
       nIndex = 3;
    } else if ( ui->chk4->isChecked( ) ) {
      nIndex = 4;
    } else if ( ui->chk5->isChecked( ) ) {
      nIndex = 5;
    } else if ( ui->chk6->isChecked( ) ) {
      nIndex = 6;
    }

    return nIndex;
}

void CDlgStaying::GetOrderByClause( QString& strOrder, int nChk, int nCb )
{
    QString strAsc = ( 0 == nCb ) ? " ASC " : " DESC ";
    strOrder = " Order by %1 ";

    switch ( nChk ) {
    case 0 :
        strOrder = strOrder.arg( "cardno" );
        break;

    case 1 :
        strOrder = strOrder.arg( "cardselfno" );
        break;

    case 2 :
        strOrder = strOrder.arg( "username" );
        break;

    case 3 :
        strOrder = strOrder.arg( "b.userphone" );
        break;

    case 4 :
        strOrder = strOrder.arg( "carcp" );
        break;

    case 5 :
        strOrder = strOrder.arg( "inshebeiname" );
        break;

    case 6 :
        strOrder = strOrder.arg( "intime" );
        break;

    default :
        break;
    }

    strOrder += strAsc;
}

void CDlgStaying::on_cbSort_currentIndexChanged(int index)
{
    SortData( GetChkIndex( ), index, true );
}

bool CDlgStaying::GetClicked( int nChk )
{
    bool bRet = bChkCliked[ nChk ];

    for ( int n = 0; n < 7; n++ ) {
        bChkCliked[ n ] = false;
    }

    bChkCliked[ nChk ] = true;

    return bRet;
}

void CDlgStaying::on_chk0_clicked(bool checked)
{
    SortData( 0, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk1_clicked(bool checked)
{
    SortData( 1, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk2_clicked(bool checked)
{
    SortData( 2, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk3_clicked(bool checked)
{
    SortData( 3, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk4_clicked(bool checked)
{
    SortData( 4, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk5_clicked(bool checked)
{
    SortData( 5, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk6_clicked(bool checked)
{
    SortData( 6, ui->cbSort->currentIndex( ) );
}

void CDlgStaying::on_chk0_toggled(bool checked)
{

}
