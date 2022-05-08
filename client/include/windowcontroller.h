//
// Created by doktorkrab on 3/11/22.
//

#ifndef FASTTYPING_WINDOWCONTROLLER_H
#define FASTTYPING_WINDOWCONTROLLER_H
#include <QMainWindow>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <string>
#include <unordered_map>

namespace FastTyping {
struct WindowController : QObject {
    Q_OBJECT
public:
    static WindowController &getInstance() {
        static WindowController controller;
        return controller;
    }

    void registerWindow(const std::string &windowName,
                        QSharedPointer<QMainWindow> window);
    void setActiveWindow(const std::string &windowName);

private:
    std::unordered_map<std::string, QSharedPointer<QMainWindow>> windows;
    QSharedPointer<QMainWindow> activeWindow;

    WindowController() = default;
};
};  // namespace FastTyping

#endif  // FASTTYPING_WINDOWCONTROLLER_H
