#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <QString>
#include <QDebug>
#include <QDateTime>

class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error,
        Database,
        Entity
    };

    static void log(Level level, const QString& message, const QString& context = "") {
        QString output;
        if (!context.isEmpty()) {
            output += QString("[%1] ").arg(context);
        }
        output += message;

        if (level == Level::Error) {
            qCritical() << output.toStdString().c_str();
        } else {
            qDebug() << output.toStdString().c_str();
        }
    }

    static void info(const QString& message, const QString& context = "") { log(Level::Info, message, context); }
    static void warn(const QString& message, const QString& context = "") { log(Level::Warning, message, context); }
    static void error(const QString& message, const QString& context = "") { log(Level::Error, message, context); }
    static void db(const QString& message, const QString& context = "") { log(Level::Database, message, context); }
    static void entity(const QString& message, const QString& context = "") { log(Level::Entity, message, context); }
};

#endif
