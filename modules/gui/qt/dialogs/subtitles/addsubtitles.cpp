/*****************************************************************************
 * addsubtitles.cpp : Add Subtitles dialog (VLC redesign)
 *****************************************************************************
 * Copyright (C) 2026 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "addsubtitles.hpp"

#include "player/player_controller.hpp"
#include "dialogs/dialogs_provider.hpp"
#include "style/widgettheme.hpp"
#include "style/vlcicons.hpp"

#include <vlc_interface.h>
#include <vlc_input_item.h>
#include <vlc_configuration.h>

#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

QStringList extensionsToFilters( const char* extensions )
{
    return QString::fromLatin1( extensions ).split( ';', Qt::SkipEmptyParts );
}

// the base name of a file, lower case: "Movie.mkv" -> "movie"
QString baseName( const QString& fileName )
{
    return QFileInfo( fileName ).completeBaseName().toLower();
}

// like the automatic subtitle detection, a subtitle matches a video when its
// name starts with the video name: "Movie.mkv" <- "Movie.en.srt", "Movie_English.srt"
bool subtitleMatches( const QString& subtitleFile, const QString& videoFile )
{
    return baseName( subtitleFile ).startsWith( baseName( videoFile ) );
}

}

AddSubtitlesDialog::AddSubtitlesDialog( qt_intf_t *_p_intf )
    : QVLCDialog( nullptr, _p_intf )
{
    setWindowTitle( qtr( "Add Subtitles" ) );
    setWindowRole( "vlc-add-subtitles" );
    resize( 1048, 800 );

    const auto c = []( const QColor& color ) { return color.name(); };

    /* Header (Figma: 96px, Dark/BG/B2): target, origin and path */
    QWidget *header = new QWidget( this );
    header->setObjectName( "vlcDialogHeader" );
    header->setAttribute( Qt::WA_StyledBackground, true );
    header->setStyleSheet( QStringLiteral(
        "QWidget#vlcDialogHeader { background: #090909; }"
        "QWidget#vlcDialogHeader QComboBox, QWidget#vlcDialogHeader QLabel#vlcPath {"
        "  background: #151515; border: none; border-radius: 6px; color: %1; min-height: 24px; padding: 5px 12px; }" )
        .arg( c( QColor( "#969696" ) ) ) );
    QHBoxLayout *headerLayout = new QHBoxLayout( header );
    headerLayout->setContentsMargins( 9, 12, 9, 12 );
    headerLayout->setSpacing( 10 );

    m_target = new QComboBox( header );
    m_target->addItem( qtr( "To This File" ), TARGET_FILE );
    m_target->addItem( qtr( "To This Folder" ), TARGET_FOLDER );
    m_target->setFixedWidth( 200 );
    headerLayout->addWidget( m_target );

    m_origin = new QComboBox( header );
    m_origin->addItem( qtr( "From Computer" ) );
    m_origin->addItem( qtr( "Choose Files..." ) );
    m_origin->setFixedWidth( 200 );
    headerLayout->addWidget( m_origin );

    m_path = new QLabel( header );
    m_path->setObjectName( "vlcPath" );
    m_path->setTextInteractionFlags( Qt::TextSelectableByMouse );
    headerLayout->addWidget( m_path, 1 );

    /* Content */
    const QString listStyle = QStringLiteral(
        "QListWidget { background: transparent; border: none; color: %1; font-size: 14px; outline: none; }"
        "QListWidget::item { min-height: 36px; padding: 0px 8px; border-radius: 6px; margin: 2px 0px; }"
        "QListWidget::item:hover { background: #151515; }"
        "QListWidget::item:selected { background: #090909; color: %1; }"
        "QListWidget::indicator { width: 16px; height: 16px; border: 1px solid #373737; border-radius: 4px; background: #090909; }"
        "QListWidget::indicator:checked { background: %2; border-color: %2; }" )
        .arg( c( QColor( "#969696" ) ), c( WidgetTheme::accent() ) );

    m_fileList = new QListWidget( this );
    m_fileList->setStyleSheet( listStyle );
    m_fileList->setIconSize( QSize( 24, 24 ) );
    connect( m_fileList, &QListWidget::itemClicked, this, []( QListWidgetItem *item ) {
        item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    } );

    m_folderList = new QListWidget( this );
    m_folderList->setStyleSheet( listStyle );
    m_folderList->setIconSize( QSize( 24, 24 ) );
    m_folderList->setSelectionMode( QAbstractItemView::NoSelection );

    m_empty = new QLabel( this );
    m_empty->setAlignment( Qt::AlignCenter );
    m_empty->setWordWrap( true );
    m_empty->setStyleSheet( QStringLiteral( "color: %1; font-size: 14px;" ).arg( c( WidgetTheme::textTertiary() ) ) );

    m_stack = new QStackedWidget( this );
    m_stack->addWidget( m_fileList );
    m_stack->addWidget( m_folderList );
    m_stack->addWidget( m_empty );

    QWidget *content = new QWidget( this );
    QVBoxLayout *contentLayout = new QVBoxLayout( content );
    contentLayout->setContentsMargins( 20, 22, 20, 0 );
    contentLayout->addWidget( m_stack );

    /* Footer (Figma: Cancel left, Save right) */
    QPushButton *cancel = new QPushButton( qtr( "Cancel" ), this );
    m_save = new QPushButton( qtr( "Save" ), this );
    m_save->setDefault( true );
    cancel->setFixedSize( 100, 32 );
    m_save->setFixedSize( 100, 32 );
    QHBoxLayout *footer = new QHBoxLayout;
    footer->setContentsMargins( 20, 20, 20, 20 );
    footer->addWidget( cancel );
    footer->addStretch( 1 );
    footer->addWidget( m_save );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );
    layout->addWidget( header );
    layout->addWidget( content, 1 );
    layout->addLayout( footer );

    connect( m_target, qOverload<int>( &QComboBox::currentIndexChanged ), this, &AddSubtitlesDialog::targetChanged );
    connect( m_origin, qOverload<int>( &QComboBox::activated ), this, &AddSubtitlesDialog::originActivated );
    connect( cancel, &QPushButton::clicked, this, &QDialog::reject );
    connect( m_save, &QPushButton::clicked, this, &AddSubtitlesDialog::save );

    refresh();
}

void AddSubtitlesDialog::refresh()
{
    m_mediaUrl.clear();
    if( input_item_t *item = THEMIM->getInput() )
    {
        char *uri = input_item_GetURI( item );
        if( uri )
        {
            QUrl url( QString::fromUtf8( uri ) );
            if( url.isLocalFile() )
                m_mediaUrl = url;
            free( uri );
        }
    }

    if( m_mediaUrl.isEmpty() )
    {
        m_path->setText( QString() );
        m_empty->setText( qtr( "Play a local video file to add subtitles to it." ) );
        m_stack->setCurrentWidget( m_empty );
        m_save->setEnabled( false );
        return;
    }

    /* "Videos / Shows / The Wand / S01" */
    const QDir dir = QFileInfo( m_mediaUrl.toLocalFile() ).absoluteDir();
    QStringList parts = QDir::toNativeSeparators( dir.absolutePath() ).split( QDir::separator(), Qt::SkipEmptyParts );
    m_path->setText( parts.mid( qMax( 0, parts.size() - 4 ) ).join( QStringLiteral( " / " ) ) );
    m_path->setToolTip( dir.absolutePath() );

    targetChanged( m_target->currentIndex() );
}

void AddSubtitlesDialog::targetChanged( int index )
{
    if( m_mediaUrl.isEmpty() )
        return;
    if( m_target->itemData( index ).toInt() == TARGET_FOLDER )
        fillFolderList();
    else
        fillFileList();
}

void AddSubtitlesDialog::addSubtitleItem( const QString& path, bool checked )
{
    const QFileInfo info( path );
    for( int i = 0; i < m_fileList->count(); ++i )
        if( m_fileList->item( i )->data( Qt::UserRole ).toString() == info.absoluteFilePath() )
            return;

    QListWidgetItem *item = new QListWidgetItem(
        WidgetTheme::glyphIcon( VLCIcons::subtitles, 24, WidgetTheme::textSecondary(), WidgetTheme::textSecondary() ),
        info.fileName(), m_fileList );
    item->setData( Qt::UserRole, info.absoluteFilePath() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
    item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
    item->setToolTip( info.absoluteFilePath() );
}

void AddSubtitlesDialog::fillFileList()
{
    m_fileList->clear();

    const QFileInfo media( m_mediaUrl.toLocalFile() );
    const QFileInfoList subtitles = media.absoluteDir().entryInfoList(
        extensionsToFilters( EXTENSIONS_SUBTITLE ), QDir::Files, QDir::Name );

    for( const QFileInfo& subtitle : subtitles )
        addSubtitleItem( subtitle.absoluteFilePath(), subtitleMatches( subtitle.fileName(), media.fileName() ) );

    if( m_fileList->count() == 0 )
    {
        m_empty->setText( qtr( "There is no subtitle file next to this media.\n"
                               "Use \"Choose Files...\" to pick subtitle files from your computer." ) );
        m_stack->setCurrentWidget( m_empty );
    }
    else
        m_stack->setCurrentWidget( m_fileList );
    m_save->setEnabled( true );
}

void AddSubtitlesDialog::fillFolderList()
{
    m_folderList->clear();

    const QDir dir = QFileInfo( m_mediaUrl.toLocalFile() ).absoluteDir();
    const QFileInfoList videos = dir.entryInfoList( extensionsToFilters( EXTENSIONS_VIDEO ), QDir::Files, QDir::Name );
    const QFileInfoList subtitles = dir.entryInfoList( extensionsToFilters( EXTENSIONS_SUBTITLE ), QDir::Files, QDir::Name );


    const QIcon videoIcon = WidgetTheme::glyphIcon( VLCIcons::topbar_video, 24,
                                                    WidgetTheme::textSecondary(), WidgetTheme::textSecondary() );
    /* Figma: "video ──────── subtitle" rows */
    for( const QFileInfo& video : videos )
    {
        QStringList matching;
        for( const QFileInfo& candidate : subtitles )
            if( subtitleMatches( candidate.fileName(), video.fileName() ) )
                matching << candidate.fileName();
        const QString subtitle = matching.isEmpty() ? qtr( "no matching subtitle" ) : matching.join( QStringLiteral( ", " ) );
        QListWidgetItem *item = new QListWidgetItem( videoIcon,
            QStringLiteral( "%1   \u2500\u2500\u2500\u2500\u2500\u2500   %2" ).arg( video.fileName(), subtitle ), m_folderList );
        item->setFlags( Qt::ItemIsEnabled );
    }

    if( m_folderList->count() == 0 )
    {
        m_empty->setText( qtr( "There is no video in this folder." ) );
        m_stack->setCurrentWidget( m_empty );
    }
    else
        m_stack->setCurrentWidget( m_folderList );
    m_save->setEnabled( true );
}

void AddSubtitlesDialog::originActivated( int index )
{
    if( index != 1 || m_mediaUrl.isEmpty() )
        return;
    m_origin->setCurrentIndex( 0 );

    const QString filter = qtr( "Subtitle Files" ) + QStringLiteral( " (%1)" )
                           .arg( QString::fromLatin1( EXTENSIONS_SUBTITLE ).replace( ';', ' ' ) );
    const QStringList files = QFileDialog::getOpenFileNames( this, qtr( "Choose subtitle files" ),
        QFileInfo( m_mediaUrl.toLocalFile() ).absolutePath(), filter );
    if( files.isEmpty() )
        return;

    m_target->setCurrentIndex( TARGET_FILE );
    for( const QString& file : files )
        addSubtitleItem( file, true );
    m_stack->setCurrentWidget( m_fileList );
}

void AddSubtitlesDialog::save()
{
    if( m_target->currentData().toInt() == TARGET_FOLDER )
    {
        /* load matching subtitle files automatically for the videos of the folder */
        config_PutInt( "sub-autodetect-file", 1 );
    }
    else
    {
        for( int i = 0; i < m_fileList->count(); ++i )
        {
            QListWidgetItem *item = m_fileList->item( i );
            if( item->checkState() != Qt::Checked )
                continue;
            const QString url = QUrl::fromLocalFile( item->data( Qt::UserRole ).toString() ).toString( QUrl::FullyEncoded );
            if( THEMIM->AddAssociatedMedia( SPU_ES, url, true, true, false ) )
                msg_Warn( p_intf, "unable to load subtitles from '%s'", qtu( url ) );
        }
    }
    accept();
}
