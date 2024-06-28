// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include <streamwindow.h>
#include <streamsession.h>
#include <avopenglwidget.h>
#include <loginpindialog.h>
#include <settings.h>

#include <QLabel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QAction>
#include <QMenu>

StreamWindow::StreamWindow(const StreamSessionConnectInfo &connect_info, QWidget *parent)
	: QMainWindow(parent),
	connect_info(connect_info)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(qApp->applicationName() + " | Stream");
		
	session = nullptr;
	av_widget = nullptr;

	try
	{
		Init();
	}
	catch(const Exception &e)
	{
		QMessageBox::critical(this, tr("Stream failed"), tr("Failed to initialize Stream Session: %1").arg(e.what()));
		close();
	}
}

StreamWindow::~StreamWindow()
{
	// make sure av_widget is always deleted before the session
	delete av_widget;
}

#include <QGuiApplication>

void StreamWindow::Init()
{
	session = new StreamSession(connect_info, this);

	connect(session, &StreamSession::SessionQuit, this, &StreamWindow::SessionQuit);
	connect(session, &StreamSession::LoginPINRequested, this, &StreamWindow::LoginPINRequested);

	const QKeySequence fullscreen_shortcut = Qt::Key_F11;
	const QKeySequence stretch_shortcut = Qt::CTRL + Qt::Key_S;
	const QKeySequence zoom_shortcut = Qt::CTRL + Qt::Key_Z;

	fullscreen_action = new QAction(tr("Fullscreen"), this);
	fullscreen_action->setCheckable(true);
	fullscreen_action->setShortcut(fullscreen_shortcut);
	addAction(fullscreen_action);
	connect(fullscreen_action, &QAction::triggered, this, &StreamWindow::ToggleFullscreen);

	if(session->GetFfmpegDecoder())
	{
		av_widget = new AVOpenGLWidget(session, this, connect_info.transform_mode);
		setCentralWidget(av_widget);

		av_widget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(av_widget, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
			av_widget->ResetMouseTimeout();

			QMenu menu(av_widget);
			menu.addAction(fullscreen_action);
			menu.addSeparator();
			menu.addAction(stretch_action);
			menu.addAction(zoom_action);
			releaseKeyboard();
			connect(&menu, &QMenu::aboutToHide, this, [this] {
				grabKeyboard();
			});
			menu.exec(av_widget->mapToGlobal(pos));
		});
	}
	else
	{
		QWidget *bg_widget = new QWidget(this);
		bg_widget->setStyleSheet("background-color: black;");
		setCentralWidget(bg_widget);
	}

	grabKeyboard();

	session->Start();

	stretch_action = new QAction(tr("Stretch"), this);
	stretch_action->setCheckable(true);
	stretch_action->setShortcut(stretch_shortcut);
	addAction(stretch_action);
	connect(stretch_action, &QAction::triggered, this, &StreamWindow::ToggleStretch);

	zoom_action = new QAction(tr("Zoom"), this);
	zoom_action->setCheckable(true);
	zoom_action->setShortcut(zoom_shortcut);
	addAction(zoom_action);
	connect(zoom_action, &QAction::triggered, this, &StreamWindow::ToggleZoom);

	auto quit_action = new QAction(tr("Quit"), this);
	quit_action->setShortcut(Qt::CTRL + Qt::Key_Q);
	addAction(quit_action);
	connect(quit_action, &QAction::triggered, this, &StreamWindow::Quit);

	resize(connect_info.video_profile.width, connect_info.video_profile.height);

	if(connect_info.fullscreen)
	{
		showFullScreen();
		fullscreen_action->setChecked(true);
	}
	else
		show();

	UpdateTransformModeActions();
}

void StreamWindow::keyPressEvent(QKeyEvent *event)
{
	if(session)
		session->HandleKeyboardEvent(event);
}

void StreamWindow::keyReleaseEvent(QKeyEvent *event)
{
	if(session)
		session->HandleKeyboardEvent(event);
}

void StreamWindow::Quit()
{
	close();
}

void StreamWindow::mousePressEvent(QMouseEvent *event)
{
	if(session && session->HandleMouseEvent(event))
		return;
	QMainWindow::mousePressEvent(event);
}

void StreamWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if(session && session->HandleMouseEvent(event))
		return;
	QMainWindow::mouseReleaseEvent(event);
}

void StreamWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(event->button() == Qt::MouseButton::LeftButton)
	{
		ToggleFullscreen();
		return;
	}
	QMainWindow::mouseDoubleClickEvent(event);
}

void StreamWindow::closeEvent(QCloseEvent *event)
{
	if(session)
	{
		if(session->IsConnected())
		{
			bool sleep = false;
			switch(connect_info.settings->GetDisconnectAction())
			{
				case DisconnectAction::Ask: {
					auto res = QMessageBox::question(this, tr("Disconnect Session"), tr("Do you want the Console to go into sleep mode?"),
							QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
					switch(res)
					{
						case QMessageBox::Yes:
							sleep = true;
							break;
						case QMessageBox::Cancel:
							event->ignore();
							return;
						default:
							break;
					}
					break;
				}
				case DisconnectAction::AlwaysSleep:
					sleep = true;
					break;
				default:
					break;
			}
			if(sleep)
				session->GoToBed();
		}
		session->Stop();
	}
}

void StreamWindow::SessionQuit(ChiakiQuitReason reason, const QString &reason_str)
{
	if(chiaki_quit_reason_is_error(reason))
	{
		QString m = tr("Chiaki Session has quit") + ":\n" + chiaki_quit_reason_string(reason);
		if(!reason_str.isEmpty())
			m += "\n" + tr("Reason") + ": \"" + reason_str + "\"";
		QMessageBox::critical(this, tr("Session has quit"), m);
	}
	close();
}

void StreamWindow::LoginPINRequested(bool incorrect)
{
	auto dialog = new LoginPINDialog(incorrect, this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
		grabKeyboard();

		if(!session)
			return;

		if(result == QDialog::Accepted)
			session->SetLoginPIN(dialog->GetPIN());
		else
			session->Stop();
	});
	releaseKeyboard();
	dialog->show();
}

void StreamWindow::ToggleFullscreen()
{
	if(isFullScreen())
	{
		showNormal();
		fullscreen_action->setChecked(false);
	}
	else
	{
		showFullScreen();
		if(av_widget)
			av_widget->HideMouse();
		fullscreen_action->setChecked(true);
	}
}

void StreamWindow::UpdateTransformModeActions()
{
	TransformMode tm = av_widget ? av_widget->GetTransformMode() : TransformMode::Fit;
	stretch_action->setChecked(tm == TransformMode::Stretch);
	zoom_action->setChecked(tm == TransformMode::Zoom);
}

void StreamWindow::ToggleStretch()
{
	if(!av_widget)
		return;
	av_widget->SetTransformMode(
			av_widget->GetTransformMode() == TransformMode::Stretch
			? TransformMode::Fit
			: TransformMode::Stretch);
	UpdateTransformModeActions();
}

void StreamWindow::ToggleZoom()
{
	if(!av_widget)
		return;
	av_widget->SetTransformMode(
			av_widget->GetTransformMode() == TransformMode::Zoom
			? TransformMode::Fit
			: TransformMode::Zoom);
	UpdateTransformModeActions();
}

void StreamWindow::resizeEvent(QResizeEvent *event)
{
	UpdateVideoTransform();
	QMainWindow::resizeEvent(event);
}

void StreamWindow::moveEvent(QMoveEvent *event)
{
	UpdateVideoTransform();
	QMainWindow::moveEvent(event);
}

void StreamWindow::changeEvent(QEvent *event)
{
	if(event->type() == QEvent::ActivationChange)
		UpdateVideoTransform();
	QMainWindow::changeEvent(event);
}

void StreamWindow::UpdateVideoTransform()
{
#if CHIAKI_LIB_ENABLE_PI_DECODER
	ChiakiPiDecoder *pi_decoder = session->GetPiDecoder();
	if(pi_decoder)
	{
		QRect r = geometry();
		chiaki_pi_decoder_set_params(pi_decoder, r.x(), r.y(), r.width(), r.height(), isActiveWindow());
	}
#endif
}
