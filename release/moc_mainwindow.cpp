/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Reader_t {
    QByteArrayData data[17];
    char stringdata0[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Reader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Reader_t qt_meta_stringdata_Reader = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Reader"
QT_MOC_LITERAL(1, 7, 8), // "finished"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 7), // "timeout"
QT_MOC_LITERAL(4, 25, 11), // "ampNotFound"
QT_MOC_LITERAL(5, 37, 14), // "connectionLost"
QT_MOC_LITERAL(6, 52, 12), // "unknownError"
QT_MOC_LITERAL(7, 65, 4), // "read"
QT_MOC_LITERAL(8, 70, 7), // "setStop"
QT_MOC_LITERAL(9, 78, 4), // "stop"
QT_MOC_LITERAL(10, 83, 9), // "setParams"
QT_MOC_LITERAL(11, 93, 5), // "capId"
QT_MOC_LITERAL(12, 99, 12), // "samplingRate"
QT_MOC_LITERAL(13, 112, 8), // "BipRange"
QT_MOC_LITERAL(14, 121, 8), // "EegRange"
QT_MOC_LITERAL(15, 130, 10), // "hexEegMask"
QT_MOC_LITERAL(16, 141, 10) // "hexBipMask"

    },
    "Reader\0finished\0\0timeout\0ampNotFound\0"
    "connectionLost\0unknownError\0read\0"
    "setStop\0stop\0setParams\0capId\0samplingRate\0"
    "BipRange\0EegRange\0hexEegMask\0hexBipMask"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Reader[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    0,   55,    2, 0x06 /* Public */,
       4,    0,   56,    2, 0x06 /* Public */,
       5,    0,   57,    2, 0x06 /* Public */,
       6,    0,   58,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   59,    2, 0x0a /* Public */,
       8,    1,   60,    2, 0x0a /* Public */,
      10,    6,   63,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Double, QMetaType::Double, QMetaType::ULongLong, QMetaType::ULongLong,   11,   12,   13,   14,   15,   16,

       0        // eod
};

void Reader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Reader *_t = static_cast<Reader *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->finished(); break;
        case 1: _t->timeout(); break;
        case 2: _t->ampNotFound(); break;
        case 3: _t->connectionLost(); break;
        case 4: _t->unknownError(); break;
        case 5: _t->read(); break;
        case 6: _t->setStop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->setParams((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< unsigned long long(*)>(_a[5])),(*reinterpret_cast< unsigned long long(*)>(_a[6]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Reader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Reader::finished)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Reader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Reader::timeout)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Reader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Reader::ampNotFound)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Reader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Reader::connectionLost)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Reader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Reader::unknownError)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Reader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Reader.data,
      qt_meta_data_Reader,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Reader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Reader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Reader.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Reader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Reader::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Reader::timeout()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Reader::ampNotFound()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Reader::connectionLost()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Reader::unknownError()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[13];
    char stringdata0[151];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 14), // "threadFinished"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 13), // "threadTimeout"
QT_MOC_LITERAL(4, 41, 14), // "connectionLost"
QT_MOC_LITERAL(5, 56, 11), // "ampNotFound"
QT_MOC_LITERAL(6, 68, 12), // "unknownError"
QT_MOC_LITERAL(7, 81, 18), // "load_config_dialog"
QT_MOC_LITERAL(8, 100, 18), // "save_config_dialog"
QT_MOC_LITERAL(9, 119, 4), // "link"
QT_MOC_LITERAL(10, 124, 10), // "closeEvent"
QT_MOC_LITERAL(11, 135, 12), // "QCloseEvent*"
QT_MOC_LITERAL(12, 148, 2) // "ev"

    },
    "MainWindow\0threadFinished\0\0threadTimeout\0"
    "connectionLost\0ampNotFound\0unknownError\0"
    "load_config_dialog\0save_config_dialog\0"
    "link\0closeEvent\0QCloseEvent*\0ev"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    0,   62,    2, 0x08 /* Private */,
       6,    0,   63,    2, 0x08 /* Private */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    1,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->threadFinished(); break;
        case 1: _t->threadTimeout(); break;
        case 2: _t->connectionLost(); break;
        case 3: _t->ampNotFound(); break;
        case 4: _t->unknownError(); break;
        case 5: _t->load_config_dialog(); break;
        case 6: _t->save_config_dialog(); break;
        case 7: _t->link(); break;
        case 8: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
