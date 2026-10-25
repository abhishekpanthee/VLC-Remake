/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "firstrunwizard.hpp"
#include "style/widgettheme.hpp"
#include "util/color_scheme_model.hpp"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHash>
#include "util/color_scheme_model.hpp"
#include "maininterface/mainctx.hpp"
#include "dialogs/toolbar/controlbar_profile_model.hpp"
#include "medialibrary/medialib.hpp"

#include <QPushButton>
#include <QButtonGroup>
#include <QFileDialog>

#include <vlc_common.h>
#include <vlc_configuration.h>
#include <vlc_url.h>
#include <vlc_cxx_helpers.hpp>

FirstRunWizard::FirstRunWizard( qt_intf_t *_p_intf, QWidget *parent)
               : QWizard( parent )
{
    p_intf = _p_intf;

    /* Set windown properties */
    setWindowTitle( qtr( "Welcome" ) );
    setWindowModality( Qt::WindowModal );

    // NOTE: Aero style seems to be incompatible with
    //       Qt Modern Windows Style dark mode
    setWizardStyle(QWizard::ModernStyle);

    /* Build the Ui */
    ui.setupUi( this );

    /* Set the privacy and network policy */
    ui.policy->setHtml( qtr( "<p>In order to protect your privacy, <i>VLC media player</i> "
        "does <b>not</b> collect personal data or transmit them, "
        "not even in anonymized form, to anyone."
        "</p>\n"
        "<p>Nevertheless, <i>VLC</i> is able to automatically retrieve "
        "information about the media in your play queue from third party "
        "Internet-based services. This includes cover art, track names, "
        "artist names and other meta-data."
        "</p>\n"
        "<p>Consequently, this may entail identifying some of your media files to third party "
        "entities. Therefore the <i>VLC</i> developers require your express "
        "consent for the media player to access the Internet automatically."
        "</p>\n" ) );
    ui.policy->setReadOnly( true );

    /* Set up the button group for colour schemes. */
    /* Creating in Qt Designer has unstable ordering when calling buttons() */
    colorSchemeGroup = new QButtonGroup( this );
    colorSchemeGroup->addButton( ui.systemButton );
    colorSchemeGroup->addButton( ui.lightButton );
    colorSchemeGroup->addButton( ui.darkButton );
    colorSchemeGroup->setExclusive( true );

    colorSchemeImages = new QButtonGroup( this );
    colorSchemeImages->addButton( ui.daynightImage );
    colorSchemeImages->addButton( ui.lightImage );
    colorSchemeImages->addButton( ui.darkImage );

    /* Setup the layout page */
    ui.layoutGroup->setId( ui.modernButton, 0 );
    ui.layoutGroup->setId( ui.classicButton, 1 );

    layoutImages = new QButtonGroup( this );
    layoutImages->addButton( ui.modernImage );
    layoutImages->addButton( ui.classicImage );
    layoutImages->setId( ui.modernImage, MODERN );
    layoutImages->setId( ui.classicImage, CLASSIC );

    /* Set the tooltips for each of the layout choices */
    ui.classicButton->setToolTip( qtr("<ul>"
                                      "<li>No client-side decoration</li>"
                                      "<li>Toolbar menu</li>"
                                      "<li>Pinned video controls</li>"
                                      "</ul>") );

    ui.modernButton->setToolTip( qtr("<ul>"
                                     "<li>Client-side decoration</li>"
                                     "<li>No toolbar menu</li>"
                                     "<li>Video controls unpinned</li>"
                                     "</ul>") );

    /* Remove the cancel button */
    setOption( QWizard::NoCancelButton );

    /* Create new instance of MLFoldersModel */
    if ( vlc_ml_instance_get( p_intf ) )
    {
        const auto foldersModel = new MLFoldersModel( this );
        foldersModel->setCtx( p_intf->p_mi );
        ui.entryPoints->setMLFoldersModel( foldersModel );
        mlFoldersEditor = ui.entryPoints;
        mlFoldersModel = foldersModel;
    }
    else
    {
        ui.enableMl->setChecked( false );
    }

    /* Disable the ML button as toggling doesn't actually do anything */
    ui.enableMl->setEnabled(false);

    /* Slots and Signals */
    connect( ui.addButton, &QPushButton::clicked, this, &FirstRunWizard::MLaddNewFolder );
    connect( colorSchemeGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::updateColorLabel );
    connect( colorSchemeImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::imageColorSchemeClick );
    connect( ui.layoutGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::updateLayoutLabel );
    connect( layoutImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ), this, &FirstRunWizard::imageLayoutClick );
    connect( this->button(QWizard::FinishButton), &QPushButton::clicked, this, &FirstRunWizard::finish );

    setupRedesign();
}

namespace {
QLabel* makeLabel( const QString& text, int pixelSize, QFont::Weight weight, const QColor& color, QWidget* parent )
{
    QLabel* label = new QLabel( text, parent );
    QFont font = label->font();
    font.setPixelSize( pixelSize );
    font.setWeight( weight );
    label->setFont( font );
    label->setStyleSheet( QStringLiteral( "color: %1;" ).arg( color.name() ) );
    label->setWordWrap( true );
    return label;
}

// Figma: Onboarding theme cards (116x116, 100x100 swatch with 12px radius)
QIcon schemeCardIcon( int scheme )
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap( QSize( 100, 100 ) * dpr );
    pixmap.setDevicePixelRatio( dpr );
    pixmap.fill( Qt::transparent );
    QPainter painter( &pixmap );
    painter.setRenderHint( QPainter::Antialiasing );
    QPainterPath path;
    path.addRoundedRect( QRectF( 0, 0, 100, 100 ), 12, 12 );
    const QColor light( "#AFAFAF" ), dark( "#222222" );
    if ( scheme == ColorSchemeModel::Day )
        painter.fillPath( path, light );
    else if ( scheme == ColorSchemeModel::Night )
        painter.fillPath( path, dark );
    else
    {
        painter.fillPath( path, light );
        painter.setClipPath( path );
        QPolygonF half; half << QPointF( 100, 0 ) << QPointF( 100, 100 ) << QPointF( 0, 100 );
        painter.setPen( Qt::NoPen );
        painter.setBrush( dark );
        painter.drawPolygon( half );
    }
    return QIcon( pixmap );
}

// Figma: Onboarding shortcut keycaps
QWidget* makeKeycaps( const QString& sequences, QWidget* parent )
{
    QWidget* row = new QWidget( parent );
    QHBoxLayout* layout = new QHBoxLayout( row );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 4 );

    const auto keyName = []( QString key ) {
        static const QHash<QString, QString> arrows {
            { "Left", QStringLiteral( "\u2190" ) }, { "Right", QStringLiteral( "\u2192" ) },
            { "Up", QStringLiteral( "\u2191" ) }, { "Down", QStringLiteral( "\u2193" ) } };
        if ( arrows.contains( key ) )
            return arrows.value( key );
        if ( key == "Space" )
            return key;
        return key.toUpper();
    };

    const QStringList alternatives = sequences.split( '|', Qt::SkipEmptyParts );
    for ( int i = 0; i < alternatives.size(); ++i )
    {
        if ( i > 0 )
        {
            QLabel* sep = new QLabel( "/", row );
            sep->setStyleSheet( QStringLiteral( "color: %1; font-size: 16px; padding: 0px 6px;" )
                                .arg( WidgetTheme::textSecondary().name() ) );
            layout->addWidget( sep );
        }
        for ( const QString& key : alternatives.at( i ).split( '+', Qt::SkipEmptyParts ) )
        {
            QLabel* cap = new QLabel( keyName( key ), row );
            cap->setAlignment( Qt::AlignCenter );
            cap->setMinimumSize( 42, 42 );
            cap->setStyleSheet( QStringLiteral(
                "QLabel { background: #1B1B1B; color: %1; border: 3px solid #373737;"
                " border-radius: 12px; padding: 4px 12px; font-size: 16px; font-weight: 300; }" )
                .arg( WidgetTheme::textPrimary().name() ) );
            layout->addWidget( cap );
        }
    }
    layout->addStretch( 1 );
    return row;
}
}

void FirstRunWizard::setupRedesign()
{
    /* VLC redesign (Figma: Onboarding) */
    setMinimumSize( 960, 680 );
    setWizardStyle( QWizard::ClassicStyle );
    setStyleSheet( QStringLiteral( "QWizard { background: #0E0E0E; }" ) );

    const QString primaryButton = QStringLiteral(
        "QPushButton { background: %1; color: #161616; font-size: 16px; font-weight: 600;"
        " border-radius: 6px; min-width: 84px; min-height: 24px; }" ).arg( WidgetTheme::accent().name() );
    button( QWizard::NextButton )->setStyleSheet( primaryButton );
    button( QWizard::FinishButton )->setStyleSheet( primaryButton );
    button( QWizard::BackButton )->setStyleSheet( QStringLiteral(
        "QPushButton { font-size: 16px; min-width: 84px; min-height: 24px; }" ) );

    /* Welcome: logo, title and description above the privacy consent */
    {
        QWidget* hero = new QWidget( ui.welcomePage );
        QVBoxLayout* heroLayout = new QVBoxLayout( hero );
        heroLayout->setSpacing( 16 );
        QLabel* logo = new QLabel( hero );
        logo->setPixmap( QIcon( QStringLiteral( ":/logo/vlc128.png" ) ).pixmap( 96, 96 ) );
        logo->setAlignment( Qt::AlignCenter );
        heroLayout->addWidget( logo );
        QLabel* title = makeLabel( qtr( "Welcome to VLC!" ), 32, QFont::Light, WidgetTheme::textPrimary(), hero );
        title->setAlignment( Qt::AlignCenter );
        heroLayout->addWidget( title );
        QLabel* description = makeLabel( qtr( "VLC is a free and open source cross-platform multimedia player and "
                                              "framework that plays most multimedia files as well as DVDs, Audio CDs, "
                                              "VCDs, and various streaming protocols." ),
                                         16, QFont::Light, QColor( "#969696" ), hero );
        description->setAlignment( Qt::AlignCenter );
        description->setFixedWidth( 595 );
        description->setMinimumHeight( description->heightForWidth( 595 ) );
        heroLayout->addWidget( description, 0, Qt::AlignHCenter );

        QVBoxLayout* pageLayout = new QVBoxLayout;
        pageLayout->addWidget( hero );
        pageLayout->addWidget( ui.horizontalFrame_policy, 1 );
        pageLayout->addWidget( ui.horizontalFrame_prefs );
        delete ui.welcomePage->layout();
        ui.welcomePage->setLayout( pageLayout );
        ui.policy->setMaximumHeight( 110 );
    }

    /* Theme: Light / Dark / System cards and accent colors */
    {
        for ( QAbstractButton* image : colorSchemeImages->buttons() )
        {
            image->setIconSize( QSize( 100, 100 ) );
            image->setFixedSize( 116, 116 );
        }
        connect( colorSchemeGroup, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ),
                 this, [this]( QAbstractButton* ) { updateSchemeCards(); } );
        connect( colorSchemeImages, qOverload<QAbstractButton*>( &QButtonGroup::buttonClicked ),
                 this, [this]( QAbstractButton* ) { updateSchemeCards(); } );

        QGridLayout* grid = qobject_cast<QGridLayout*>( ui.colorSchemePage->layout() );
        if ( grid )
        {
            QWidget* accents = new QWidget( ui.colorSchemePage );
            QHBoxLayout* accentsLayout = new QHBoxLayout( accents );
            accentsLayout->setSpacing( 12 );
            accentsLayout->addStretch( 1 );
            accentGroup = new QButtonGroup( this );
            accentGroup->setExclusive( true );
            const QStringList swatches { "#D37918", "#E15032", "#C49E23", "#19A234", "#18AAD3",
                                         "#185DD3", "#8E42FF", "#F052B3", "#AFAFAF" };
            for ( int i = 0; i < swatches.size(); ++i )
            {
                QToolButton* swatch = new QToolButton( accents );
                swatch->setCheckable( true );
                swatch->setFixedSize( 40, 40 );
                swatch->setToolTip( swatches.at( i ) );
                swatch->setStyleSheet( QStringLiteral(
                    "QToolButton { background: %1; border-radius: 20px; border: 3px solid transparent; }"
                    "QToolButton:checked { border: 3px solid #EAEAEA; }" ).arg( swatches.at( i ) ) );
                accentGroup->addButton( swatch, i );
                accentsLayout->addWidget( swatch );
                if ( i == 0 )
                    swatch->setChecked( true );
            }
            accentsLayout->addStretch( 1 );
            connect( accentGroup, &QButtonGroup::idClicked, this, [this, swatches]( int id ) {
                chosenAccent = ( id == 0 ) ? QString() : swatches.at( id );
            } );
            QLabel* hint = makeLabel( qtr( "Accent color (applied after a restart)" ), 14, QFont::Normal,
                                      WidgetTheme::textSecondary(), ui.colorSchemePage );
            hint->setAlignment( Qt::AlignCenter );
            const int row = grid->rowCount();
            grid->addWidget( hint, row, 0, 1, grid->columnCount() );
            grid->addWidget( accents, row + 1, 0, 1, grid->columnCount() );
        }
    }

    /* Shortcuts */
    setPage( SHORTCUTS_PAGE, createShortcutsPage() );
}

void FirstRunWizard::updateSchemeCards()
{
    const int checked = colorSchemeGroup->checkedId();
    for ( QAbstractButton* image : colorSchemeImages->buttons() )
    {
        const int scheme = colorSchemeImages->id( image );
        image->setIcon( schemeCardIcon( scheme ) );
        image->setStyleSheet( QStringLiteral( "QPushButton { background: transparent; border-radius: 19px;"
                                              " border: 4px solid %1; padding: 0px; }" )
                              .arg( scheme == checked ? WidgetTheme::accent().name() : QStringLiteral( "transparent" ) ) );
    }
}

QWizardPage* FirstRunWizard::createShortcutsPage()
{
    QWizardPage* page = new QWizardPage( this );
    QVBoxLayout* layout = new QVBoxLayout( page );
    layout->setSpacing( 24 );

    QLabel* title = makeLabel( qtr( "Learn the basic keyboard shortcuts." ), 24, QFont::Normal, QColor( "#969696" ), page );
    title->setAlignment( Qt::AlignCenter );
    layout->addWidget( title );

    const auto hotkey = [this]( const char* option ) -> QString {
        char* value = var_InheritString( p_intf, option );
        const QString text = value ? QString::fromUtf8( value ) : QString();
        free( value );
        return text.split( '\t' ).value( 0 );
    };
    const auto pair = [&]( const char* a, const char* b ) -> QString {
        const QString first = hotkey( a ), second = hotkey( b );
        if ( first.isEmpty() || second.isEmpty() )
            return QString( first + second );
        return QString( first + '|' + second );
    };

    const QList<std::pair<QString, QString>> left {
        { qtr( "Play / Pause" ), hotkey( "key-play-pause" ) },
        { qtr( "Seek Backward / Forward" ), pair( "key-jump-short", "key-jump+short" ) },
        { qtr( "Previous / Next Item" ), pair( "key-prev", "key-next" ) },
        { qtr( "Next Frame" ), hotkey( "key-frame-next" ) },
        { qtr( "Cycle Subtitle Track" ), hotkey( "key-subtitle-track" ) },
        { qtr( "Cycle Audio Track" ), hotkey( "key-audio-track" ) } };
    const QList<std::pair<QString, QString>> right {
        { qtr( "Toggle Fullscreen" ), hotkey( "key-toggle-fullscreen" ) },
        { qtr( "Increase / Decrease Volume" ), pair( "key-vol-up", "key-vol-down" ) },
        { qtr( "Take a Snapshot" ), hotkey( "key-snapshot" ) },
        { qtr( "Record" ), hotkey( "key-record" ) } };

    QHBoxLayout* columns = new QHBoxLayout;
    columns->setSpacing( 80 );
    for ( const auto* list : { &left, &right } )
    {
        QGridLayout* grid = new QGridLayout;
        grid->setHorizontalSpacing( 20 );
        grid->setVerticalSpacing( 14 );
        int row = 0;
        for ( const auto& [action, keys] : *list )
        {
            if ( keys.isEmpty() )
                continue;
            grid->addWidget( makeLabel( action, 16, QFont::Light, QColor( "#969696" ), page ), row, 0 );
            grid->addWidget( makeKeycaps( keys, page ), row, 1 );
            ++row;
        }
        grid->setRowStretch( row, 1 );
        columns->addLayout( grid );
    }
    layout->addLayout( columns, 1 );

    layout->addWidget( makeLabel( qtr( "Find the full list in the menu." ), 16, QFont::Normal,
                                  WidgetTheme::textTertiary(), page ) );
    return page;
}

/**
 * Function called when the finish button is pressed.
 * Processes all inputted settings according to chosen options
 */
void FirstRunWizard::finish()
{
    /* Welcome Page settings  */
    config_PutInt( "metadata-network-access", ui.privacyCheckbox->isChecked() );
    config_PutInt( "qt-privacy-ask", 0 );

    /* Colour Page settings */
    p_intf->p_mi->getColorScheme()->setCurrentIndex( colorSchemeGroup->checkedId() );
    if( !chosenAccent.isEmpty() )
        config_PutPsz( "qt-accent-color", qtu( chosenAccent ) );

    /* Layout Page settings */
    config_PutInt( "qt-menubar", ui.layoutGroup->checkedId() );
    config_PutInt( "qt-titlebar", ui.layoutGroup->checkedId() );

    config_PutInt( "qt-pin-controls", ui.layoutGroup->checkedId() );

    {
        ControlbarProfileModel* controlbarModel = p_intf->p_mi->controlbarProfileModel();
        assert(controlbarModel);

        ControlbarProfileModel::Style style;
        if( ui.layoutGroup->checkedId() )
            style = ControlbarProfileModel::Style::CLASSIC_STYLE;
        else
            style = ControlbarProfileModel::Style::DEFAULT_STYLE;

        std::unique_ptr<ControlbarProfile> profile( controlbarModel->generateProfileFromStyle( style ) );
        assert( profile );
        const std::optional<int> index = controlbarModel->findModel( profile.get() );
        if( index )
            controlbarModel->setSelectedProfile( *index );
        else
            controlbarModel->insertProfile( std::move( profile ), true );
    }

    /* Commit changes to the scanned folders for the Media Library */
    if( vlc_ml_instance_get( p_intf ) && mlFoldersEditor )
        mlFoldersEditor->commit();

    /* Reload the indexing service for the media library */
    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();
    config_SaveConfigFile( p_intf );
}

/**
 * Opens a QFileDialog for the user to select a folder to add to the Media Library
 * If a folder is selected, add it to the list of folders under MLFoldersEditor
 */
void FirstRunWizard::MLaddNewFolder()
{
    QUrl newEntryPoint = QFileDialog::getExistingDirectoryUrl( this, qtr("Choose a folder to add to the Media Library"),
                                                               QUrl( QDir::homePath() ));

    if( !newEntryPoint.isEmpty() && !mlFoldersEditor->contains( newEntryPoint ) )
        mlFoldersEditor->add( newEntryPoint );
}

/**
 * Automatically updates the label on the color scheme page depending
 * on the currently selected radio button choice
 * @param id The id of the button we are updating the label of
 */
void FirstRunWizard::updateColorLabel( QAbstractButton* btn )
{
    switch ( colorSchemeGroup->id(btn) )
    {
        case ColorSchemeModel::System:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically switch to dark mode accordingly with system settings</i>" ) );
            break;
        case ColorSchemeModel::Day:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically use light mode</i>" ) );
            break;
        case ColorSchemeModel::Night:
            ui.explainerLabel->setText( qtr( "<i>VLC will automatically use dark mode</i>" ) );
            break;
    }
}

/**
 * Automatically updates the label depending on the currently selected layout
 * @param id The id of the button we are updating the label of
 */
void FirstRunWizard::updateLayoutLabel( QAbstractButton* btn )
{
    switch ( ui.layoutGroup->id( btn ) )
    {
        case MODERN:
            ui.layoutExplainer->setText( qtr( "<i>VLC will use a modern layout with no menubar or pinned controls but with client-side decoration</i>" ) );
            break;
        case CLASSIC:
            ui.layoutExplainer->setText( qtr( "<i>VLC will use a classic layout with a menubar and pinned controls but with no client-side decoration</i>" ) );
            break;
    }
}

/**
 * Checks the correct button when the corresponding image is clicked
 * for the color scheme page.
 * @param id The id of the image that was clicked
 */
void FirstRunWizard::imageColorSchemeClick( QAbstractButton* btn )
{
    QAbstractButton* groupBtn = colorSchemeGroup->buttons().at( colorSchemeImages->id( btn ) );
    assert( groupBtn );
    groupBtn->setChecked( true );
    updateColorLabel( groupBtn );
}

/**
 * Checks the correct button when the corresponding image is clicked
 * for the layouts page.
 * @param id The id of the image that was clicked
 */
void FirstRunWizard::imageLayoutClick( QAbstractButton* btn )
{
    QAbstractButton* layoutBtn = ui.layoutGroup->buttons().at( layoutImages->id( btn ) );
    assert( layoutBtn );
    layoutBtn->setChecked( true );
    updateLayoutLabel( layoutBtn );
}

/**
 * Defines the navigation of pages for the wizard
 * @return int - the page id to go to or -1 if we are done
 */
int FirstRunWizard::nextId() const
{
    switch ( currentId() )
    {
        case WELCOME_PAGE:
            if( ui.enableMl->isChecked() )
                return FOLDER_PAGE;
            else
                return COLOR_SCHEME_PAGE;
        case FOLDER_PAGE:
            return COLOR_SCHEME_PAGE;
        case COLOR_SCHEME_PAGE:
            return LAYOUT_PAGE;
        case LAYOUT_PAGE:
            return SHORTCUTS_PAGE;
        default:
            return -1;
    }
}

/**
 * Sets up the buttons and options for the color scheme page
 * Needs to be set up later or else the main interface hasn't been created yet
 * Color schemes are always in the order system/auto, day then night
 */
void FirstRunWizard::initializePage( int id )
{
    if(id == COLOR_SCHEME_PAGE)
    {
        QVector<ColorSchemeModel::Item> schemes = p_intf->p_mi->getColorScheme()->getSchemes();
        auto schemeButtons = colorSchemeGroup->buttons();
        auto schemeImages = colorSchemeImages->buttons();

        /* Number of buttons should be equal to the schemes we can choose from */
        assert( schemes.size() == schemeButtons.size() );
        assert( schemes.size() == schemeImages.size() );

        for( int i = 0; i < schemes.size(); i++ )
        {
            colorSchemeGroup->setId( schemeButtons.at(i), schemes.at(i).scheme );
            colorSchemeImages->setId( schemeImages.at(i), schemes.at(i).scheme );
            schemeButtons.at(i)->setText( schemes.at(i).text );
            if( !i ) updateColorLabel( schemeButtons.at(i) );
        }
        updateSchemeCards();
    }
    else if ( id == FOLDER_PAGE )
        addDefaults();
}

/**
 * Processes the default options on rejection of the FirstRun Wizard.
 * The default options are:
 * - Yes to metadata
 * - System/Auto colour scheme
 * - Modern VLC layout
 */
void FirstRunWizard::reject()
{
    assert(p_intf->p_mi);
    /* Welcome Page settings  */
    config_PutInt( "metadata-network-access", 1 );
    config_PutInt( "qt-privacy-ask", 0 );

    /* Colour Page settings */
    p_intf->p_mi->getColorScheme()->setCurrentIndex( 0 );

    /* Layout Page settings */
    config_PutInt( "qt-menubar", 0 );
    config_PutInt( "qt-titlebar", 0 );
    p_intf->p_mi->setPinVideoControls( 0 );

    if( p_intf->p_mi->getMediaLibrary() )
        p_intf->p_mi->getMediaLibrary()->reload();

    config_SaveConfigFile( p_intf );
    p_intf->p_mi->reloadPrefs();
    p_intf->p_mi->controlbarProfileModel()->save();

    done( QDialog::Rejected );
}

/**
 * Adds the default folders to the media library
 */
void FirstRunWizard::addDefaults()
{
    // Return if we already set the defaults or something is null
    if( mlDefaults || !mlFoldersEditor || mlFoldersEditor->rowCount() )
        return;

    for( auto&& target : { VLC_VIDEOS_DIR, VLC_MUSIC_DIR } )
    {
        auto folder = vlc::wrap_cptr( config_GetUserDir( target ) );
        if( folder == nullptr )
            continue;
        auto folderMrl = vlc::wrap_cptr( vlc_path2uri( folder.get(), nullptr ) );
        if ( !mlFoldersEditor->contains( QUrl( folderMrl.get() ) ) )
            mlFoldersEditor->add( QUrl( folderMrl.get() ) );
    }

    mlDefaults = true;
}
