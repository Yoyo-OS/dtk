#include "dmessagemanager.h"
#include <QDebug>
#include <QEvent>

#define D_MESSAGE_MANAGER_CONTENT "_d_message_manager_content"

Q_DECLARE_METATYPE(QMargins)

DMessageManager::DMessageManager()               //私有静态构造函数
{
}

/*!
 * \~chinese \brief DMessageManager::instance 构造一个单例对象
 * \~chinese \return 返回对象引用
 */
DMessageManager *DMessageManager::instance()  //公有静态函数
{
    static DMessageManager instance;             //局部静态变量
    return &instance;
}

/*!
 * \~chinese \brief DMessageManager::sendMessage 发送弹窗消息
 * \~chinese \param par 父控件
 * \~chinese \param floMsg 弹窗浮动消息（控件）
 */
void DMessageManager::sendMessage(QWidget *par, DFloatingMessage *floMsg)
{
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT);

    if (!content) {
        content = new QWidget(par);
        content->setObjectName(D_MESSAGE_MANAGER_CONTENT);
        content->setAttribute(Qt::WA_AlwaysStackOnTop);

        QMargins magins = par->property("_d_margins").value<QMargins>();
        if (par->property("_d_margins").isValid())
            content->setContentsMargins(magins);
        else
            content->setContentsMargins(QMargins(20, 0, 20, 0));

        content->installEventFilter(this);
        par->installEventFilter(this);
        QVBoxLayout *layout = new QVBoxLayout(content);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->setDirection(QBoxLayout::BottomToTop);
        content->show();
    }

    static_cast<QBoxLayout*>(content->layout())->addWidget(floMsg, 0, Qt::AlignHCenter);
}

/*!
 * \~chinese \brief DMessageManager::sendMessage 发送弹窗消息
 * \~chinese \param par 父控件
 * \~chinese \param icon 消息图标
 * \~chinese \param message 消息文字
 */
void DMessageManager::sendMessage(QWidget *par, const QIcon &icon, const QString &message)
{
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT);
    int text_message_count = 0;

    for (DFloatingMessage *message : content->findChildren<DFloatingMessage*>()) {
        if (message->messageType() == DFloatingMessage::TransientType) {
            ++text_message_count;
        }
    }

    // TransientType 类型的通知消息，最多只允许同时显示三个
    if (text_message_count >= 3)
        return;

    DFloatingMessage *floMsg = new DFloatingMessage(DFloatingMessage::TransientType);
    floMsg->setAttribute(Qt::WA_DeleteOnClose);
    floMsg->setIcon(icon);
    floMsg->setMessage(message);
    sendMessage(par, floMsg);
}

/*!
 * \~chinese \brief DMessageManager::setContentMargens 设置中间间隔值
 * \~chinese \param par 父控件
 * \~chinese \param margins四周的间隔值
 * \~chinese \return 是否设置成功
 */
bool DMessageManager::setContentMargens(QWidget *par, const QMargins &margins)
{
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT);
    if (content) {
        content->setContentsMargins(margins);
        return true;
    } else {
        par->setProperty("_d_margins", QVariant::fromValue(margins));
        return false;
    }
}

/*!
 * \~chinese \brief DMessageManager::eventFilter 消息过滤
 * \~chinese \param watched 查看对象
 * \~chinese \param event 消息事件
 * \~chinese \return 成功与否
 */
bool DMessageManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Resize) {
        if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
            QWidget *content;

            if (widget->objectName() == D_MESSAGE_MANAGER_CONTENT) {
                content = widget;
            } else {
                content = widget->findChild<QWidget*>(D_MESSAGE_MANAGER_CONTENT);
            }

            QWidget *par = content->parentWidget();

            // 限制通知消息的最大宽度
            for (DFloatingMessage *message : content->findChildren<DFloatingMessage*>()) {
                message->setMaximumWidth(par->rect().marginsRemoved(content->contentsMargins()).width());
                message->setMinimumHeight(message->sizeHint().height());
            }

            QRect geometry(QPoint(0, 0), content->sizeHint());
            geometry.moveCenter(par->rect().center());
            geometry.moveBottom(par->rect().bottom());
            content->setGeometry(geometry);
        }
    } else if (event->type() == QEvent::ChildRemoved) {
        // 如果是通知消息被删除的事件
        if (QWidget *widget = qobject_cast<QWidget*>(watched)) {
            if (widget->objectName() == D_MESSAGE_MANAGER_CONTENT
                    && widget->layout() && widget->layout()->count() == 0) {
                widget->parent()->removeEventFilter(this);
                widget->deleteLater();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
