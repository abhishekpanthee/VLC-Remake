/*****************************************************************************
 * addsubtitles.hpp : Add Subtitles dialog (VLC redesign)
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
#ifndef QVLC_ADDSUBTITLES_DIALOG_H_
#define QVLC_ADDSUBTITLES_DIALOG_H_ 1

#include "widgets/native/qvlcframe.hpp"

#include <QUrl>

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QStackedWidget;

/**
 * Figma: "Add Subtitles to Playlist/Folder".
 *
 * "To This File": the subtitle files next to the playing media (and the ones
 * picked from the computer), checked ones are attached to the playing media.
 * "To This Folder": every video of the folder with its matching subtitle file,
 * saving enables the automatic loading of matching subtitle files.
 */
class AddSubtitlesDialog : public QVLCDialog
{
    Q_OBJECT

public:
    explicit AddSubtitlesDialog( qt_intf_t *_p_intf );
    ~AddSubtitlesDialog() override = default;

    /** reload the content for the media currently playing */
    void refresh();

private slots:
    void targetChanged( int index );
    void originActivated( int index );
    void save();

private:
    enum Target { TARGET_FILE = 0, TARGET_FOLDER = 1 };

    void fillFileList();
    void fillFolderList();
    void addSubtitleItem( const QString& path, bool checked );

    QComboBox *m_target = nullptr;
    QComboBox *m_origin = nullptr;
    QLabel *m_path = nullptr;
    QStackedWidget *m_stack = nullptr;
    QListWidget *m_fileList = nullptr;
    QListWidget *m_folderList = nullptr;
    QLabel *m_empty = nullptr;
    QPushButton *m_save = nullptr;

    QUrl m_mediaUrl; // file:// URL of the playing media, empty otherwise
};

#endif
