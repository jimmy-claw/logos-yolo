/****************************************************************************
** Meta object code from reading C++ file 'yolo_ui_component.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/yolo_ui_component.h"
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'yolo_ui_component.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN15YoloUIComponentE_t {};
} // unnamed namespace

template <> constexpr inline auto YoloUIComponent::qt_create_metaobjectdata<qt_meta_tag_ZN15YoloUIComponentE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "YoloUIComponent"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<YoloUIComponent, qt_meta_tag_ZN15YoloUIComponentE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject YoloUIComponent::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15YoloUIComponentE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15YoloUIComponentE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15YoloUIComponentE_t>.metaTypes,
    nullptr
} };

void YoloUIComponent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<YoloUIComponent *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *YoloUIComponent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *YoloUIComponent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15YoloUIComponentE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "IComponent"))
        return static_cast< IComponent*>(this);
    if (!strcmp(_clname, "com.logos.component.IComponent"))
        return static_cast< IComponent*>(this);
    return QObject::qt_metacast(_clname);
}

int YoloUIComponent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_YoloUIComponent[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x1e,  'c',  'o',  'm',  '.',  'l', 
    'o',  'g',  'o',  's',  '.',  'c',  'o',  'm', 
    'p',  'o',  'n',  'e',  'n',  't',  '.',  'I', 
    'C',  'o',  'm',  'p',  'o',  'n',  'e',  'n', 
    't', 
    // "className"
    0x03,  0x6f,  'Y',  'o',  'l',  'o',  'U',  'I', 
    'C',  'o',  'm',  'p',  'o',  'n',  'e',  'n', 
    't', 
    // "MetaData"
    0x04,  0xaa,  0x66,  'a',  'u',  't',  'h',  'o', 
    'r',  0x6a,  'J',  'i',  'm',  'm',  'y',  ' ', 
    'C',  'l',  'a',  'w',  0x68,  'c',  'a',  't', 
    'e',  'g',  'o',  'r',  'y',  0x66,  's',  'o', 
    'c',  'i',  'a',  'l',  0x6c,  'd',  'e',  'p', 
    'e',  'n',  'd',  'e',  'n',  'c',  'i',  'e', 
    's',  0x80,  0x6b,  'd',  'e',  's',  'c',  'r', 
    'i',  'p',  't',  'i',  'o',  'n',  0x78,  0x1d, 
    'Y',  'O',  'L',  'O',  ' ',  '-',  ' ',  'Y', 
    'o',  'u',  'r',  ' ',  'O',  'w',  'n',  ' ', 
    'L',  'o',  'c',  'a',  'l',  ' ',  'O',  'p', 
    'i',  'n',  'i',  'o',  'n',  0x64,  'i',  'c', 
    'o',  'n',  0x70,  ':',  '/',  'i',  'c',  'o', 
    'n',  's',  '/',  'y',  'o',  'l',  'o',  '.', 
    'p',  'n',  'g',  0x64,  'm',  'a',  'i',  'n', 
    0xa6,  0x6c,  'd',  'a',  'r',  'w',  'i',  'n', 
    '-',  'a',  'r',  'm',  '6',  '4',  0x6a,  'y', 
    'o',  'l',  'o',  '.',  'd',  'y',  'l',  'i', 
    'b',  0x6d,  'l',  'i',  'n',  'u',  'x',  '-', 
    'a',  'a',  'r',  'c',  'h',  '6',  '4',  0x67, 
    'y',  'o',  'l',  'o',  '.',  's',  'o',  0x6b, 
    'l',  'i',  'n',  'u',  'x',  '-',  'a',  'm', 
    'd',  '6',  '4',  0x67,  'y',  'o',  'l',  'o', 
    '.',  's',  'o',  0x6f,  'l',  'i',  'n',  'u', 
    'x',  '-',  'a',  'm',  'd',  '6',  '4',  '-', 
    'd',  'e',  'v',  0x67,  'y',  'o',  'l',  'o', 
    '.',  's',  'o',  0x6c,  'l',  'i',  'n',  'u', 
    'x',  '-',  'x',  '8',  '6',  '_',  '6',  '4', 
    0x67,  'y',  'o',  'l',  'o',  '.',  's',  'o', 
    0x70,  'l',  'i',  'n',  'u',  'x',  '-',  'x', 
    '8',  '6',  '_',  '6',  '4',  '-',  'd',  'e', 
    'v',  0x67,  'y',  'o',  'l',  'o',  '.',  's', 
    'o',  0x6f,  'm',  'a',  'n',  'i',  'f',  'e', 
    's',  't',  'V',  'e',  'r',  's',  'i',  'o', 
    'n',  0x65,  '0',  '.',  '1',  '.',  '0',  0x64, 
    'n',  'a',  'm',  'e',  0x64,  'y',  'o',  'l', 
    'o',  0x64,  't',  'y',  'p',  'e',  0x62,  'u', 
    'i',  0x67,  'v',  'e',  'r',  's',  'i',  'o', 
    'n',  0x65,  '0',  '.',  '1',  '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(YoloUIComponent, YoloUIComponent, qt_pluginMetaDataV2_YoloUIComponent)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_YoloUIComponent[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x1e,  'c',  'o',  'm',  '.',  'l', 
    'o',  'g',  'o',  's',  '.',  'c',  'o',  'm', 
    'p',  'o',  'n',  'e',  'n',  't',  '.',  'I', 
    'C',  'o',  'm',  'p',  'o',  'n',  'e',  'n', 
    't', 
    // "className"
    0x03,  0x6f,  'Y',  'o',  'l',  'o',  'U',  'I', 
    'C',  'o',  'm',  'p',  'o',  'n',  'e',  'n', 
    't', 
    // "MetaData"
    0x04,  0xaa,  0x66,  'a',  'u',  't',  'h',  'o', 
    'r',  0x6a,  'J',  'i',  'm',  'm',  'y',  ' ', 
    'C',  'l',  'a',  'w',  0x68,  'c',  'a',  't', 
    'e',  'g',  'o',  'r',  'y',  0x66,  's',  'o', 
    'c',  'i',  'a',  'l',  0x6c,  'd',  'e',  'p', 
    'e',  'n',  'd',  'e',  'n',  'c',  'i',  'e', 
    's',  0x80,  0x6b,  'd',  'e',  's',  'c',  'r', 
    'i',  'p',  't',  'i',  'o',  'n',  0x78,  0x1d, 
    'Y',  'O',  'L',  'O',  ' ',  '-',  ' ',  'Y', 
    'o',  'u',  'r',  ' ',  'O',  'w',  'n',  ' ', 
    'L',  'o',  'c',  'a',  'l',  ' ',  'O',  'p', 
    'i',  'n',  'i',  'o',  'n',  0x64,  'i',  'c', 
    'o',  'n',  0x70,  ':',  '/',  'i',  'c',  'o', 
    'n',  's',  '/',  'y',  'o',  'l',  'o',  '.', 
    'p',  'n',  'g',  0x64,  'm',  'a',  'i',  'n', 
    0xa6,  0x6c,  'd',  'a',  'r',  'w',  'i',  'n', 
    '-',  'a',  'r',  'm',  '6',  '4',  0x6a,  'y', 
    'o',  'l',  'o',  '.',  'd',  'y',  'l',  'i', 
    'b',  0x6d,  'l',  'i',  'n',  'u',  'x',  '-', 
    'a',  'a',  'r',  'c',  'h',  '6',  '4',  0x67, 
    'y',  'o',  'l',  'o',  '.',  's',  'o',  0x6b, 
    'l',  'i',  'n',  'u',  'x',  '-',  'a',  'm', 
    'd',  '6',  '4',  0x67,  'y',  'o',  'l',  'o', 
    '.',  's',  'o',  0x6f,  'l',  'i',  'n',  'u', 
    'x',  '-',  'a',  'm',  'd',  '6',  '4',  '-', 
    'd',  'e',  'v',  0x67,  'y',  'o',  'l',  'o', 
    '.',  's',  'o',  0x6c,  'l',  'i',  'n',  'u', 
    'x',  '-',  'x',  '8',  '6',  '_',  '6',  '4', 
    0x67,  'y',  'o',  'l',  'o',  '.',  's',  'o', 
    0x70,  'l',  'i',  'n',  'u',  'x',  '-',  'x', 
    '8',  '6',  '_',  '6',  '4',  '-',  'd',  'e', 
    'v',  0x67,  'y',  'o',  'l',  'o',  '.',  's', 
    'o',  0x6f,  'm',  'a',  'n',  'i',  'f',  'e', 
    's',  't',  'V',  'e',  'r',  's',  'i',  'o', 
    'n',  0x65,  '0',  '.',  '1',  '.',  '0',  0x64, 
    'n',  'a',  'm',  'e',  0x64,  'y',  'o',  'l', 
    'o',  0x64,  't',  'y',  'p',  'e',  0x62,  'u', 
    'i',  0x67,  'v',  'e',  'r',  's',  'i',  'o', 
    'n',  0x65,  '0',  '.',  '1',  '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(YoloUIComponent, YoloUIComponent)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
