#include "inputservice.h"
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#ifdef USE_IBUS
#include <ibus.h>
#endif

InputService* g_input_service = NULL;

#ifdef USE_IBUS

IBusEngine* g_bus_engine = NULL;
IBusEngineClass* g_parent_engine_class = NULL;
#define IBUS_TYPE_UNISPY_SYMBOLS_ENGINE (ibus_unispy_symbols_engine_get_type())

struct _IBusUnispySymbolsEngineClass {
    IBusEngineClass parent;
};

struct _IBusUnispySymbolsEngine
{
	IBusEngine parent;
};

typedef _IBusUnispySymbolsEngineClass IBusUnispySymbolsEngineClass;
typedef _IBusUnispySymbolsEngine IBusUnispySymbolsEngine;

GType ibus_unispy_symbols_engine_get_type(void);
static void ibus_unispy_symbols_engine_class_init (IBusUnispySymbolsEngineClass *klass);
static void ibus_unispy_symbols_engine_init(IBusEngine* engine);

//G_DEFINE_TYPE (IBusUnispySymbolsEngine, ibus_unispy_symbols_engine, IBUS_TYPE_ENGINE)

GType ibus_unispy_symbols_engine_get_type(void)
{
	qWarning() << "ibus_unispy_symbols_engine_get_type";
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusUnispySymbolsEngineClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_unispy_symbols_engine_class_init,
        NULL,
        NULL,
        sizeof (IBusUnispySymbolsEngine),
        0,
        (GInstanceInitFunc) ibus_unispy_symbols_engine_init,
    };

	if (type == 0)
	{
		type = g_type_register_static(IBUS_TYPE_ENGINE,
									"IBusUnispySymbolsEngine",
									&type_info,
									(GTypeFlags)0);
	}

	qWarning() << type;

    return type;
}

void ibus_unispy_symbols_engine_enable(IBusEngine* engine)
{
	IBusUnispySymbolsEngine* pyengine = (IBusUnispySymbolsEngine*)engine;
	g_parent_engine_class->enable(engine);
}

void ibus_unispy_symbols_engine_disable(IBusEngine* engine)
{
	IBusUnispySymbolsEngine* pyengine = (IBusUnispySymbolsEngine*)engine;
	g_parent_engine_class->disable(engine);
}

static void ibus_unispy_symbols_engine_class_init(IBusUnispySymbolsEngineClass *klass)
{
	qWarning() << "ibus_unispy_symbols_engine_class_init";
	IBusObjectClass* ibus_object_class = IBUS_OBJECT_CLASS(klass);
    IBusEngineClass* engine_class = IBUS_ENGINE_CLASS(klass);
	g_parent_engine_class = (IBusEngineClass *) g_type_class_peek_parent (klass);

	engine_class->enable = ibus_unispy_symbols_engine_enable;
    engine_class->disable = ibus_unispy_symbols_engine_disable;
}

static void ibus_unispy_symbols_engine_init(IBusEngine* engine)
{
	qWarning() << "ibus_unispy_symbols_engine_init";
    g_bus_engine = engine;
}

#endif

InputService::InputService()
{
}

InputService::~InputService()
{
}

void InputService::Init()
{

}

void InputService::UnInit()
{
}

void InputService::InputString(const QString& str)
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.symbols", "input");
    msg << str;
    QDBusConnection::sessionBus().send(msg);
}


#ifdef USE_IBUS

class IBusInputService
	: public InputService
{
public:
	virtual void Init()
	{
		qWarning() << "IBusInputService Init";
        ibus_init();
        m_bus = ibus_bus_new();
        g_object_ref_sink(m_bus);

        if (!ibus_bus_is_connected(m_bus))
        {
			qWarning() << "ibus can't connect";
            return;
        }

        m_factory = ibus_factory_new(ibus_bus_get_connection(m_bus));
        g_object_ref_sink(m_factory);

        ibus_factory_add_engine(m_factory, "unispy-symbols", IBUS_TYPE_UNISPY_SYMBOLS_ENGINE);
		ibus_factory_create_engine(m_factory, "unispy-symbols");

		// IBusEngineDesc* desc = ibus_bus_get_global_engine(m_bus);
		ibus_bus_set_global_engine(m_bus, "sunpinyin");
		
	}

	virtual void UnInit()
	{
		g_object_unref(m_factory);
        g_object_unref(m_bus);
	}

	virtual void InputString(const QString& str)
	{
		qWarning() << str;
		IBusText* text = ibus_text_new_from_string((const gchar*)str.toStdString().c_str());
		g_object_ref_sink(text);
        ibus_engine_commit_text (g_bus_engine, text);
		g_object_unref(text);
	}

private:
    IBusBus* m_bus;
    IBusFactory* m_factory;
};

#endif


InputService* GetInputServiceInstance()
{
	if (!g_input_service)
	{
#ifdef USE_IBUS
		g_input_service = new IBusInputService();
#else
        g_input_service = new InputService();
#endif
	}

	return g_input_service;
}

void FreeInputServiceInstance(InputService* service)
{
	delete service;
	service = NULL;
}





