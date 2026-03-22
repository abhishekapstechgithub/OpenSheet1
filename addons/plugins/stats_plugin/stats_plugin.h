// stats_plugin.h
// C++ plugin skeleton that adds extended statistical formulas.
// Compile as a shared library and place the .so/.dll in addons/plugins/.

#pragma once
#include <QObject>
#include "../../plugins/plugin_manager.h"

class StatsPlugin : public QObject, public OpenSheet::IPlugin {
    Q_OBJECT
    Q_INTERFACES(OpenSheet::IPlugin)
    Q_PLUGIN_METADATA(IID "io.opensheet.IPlugin/1.0")

public:
    QString name()        const override { return "Statistics Plugin"; }
    QString version()     const override { return "1.0.0"; }
    QString description() const override {
        return "Adds GEOMEAN, HARMEAN, SKEW, KURT, ZTEST, TTEST formulas.";
    }
    QString author()      const override { return "OpenSheet Contributors"; }

    bool initialize(OpenSheet::PluginContext *ctx) override;
    void shutdown() override;

private:
    // Formula implementations
    static QVariant fnGeoMean(const QVector<QVariant> &args,
                               OpenSheet::ParseContext &ctx);
    static QVariant fnHarMean(const QVector<QVariant> &args,
                               OpenSheet::ParseContext &ctx);
    static QVariant fnSkew(const QVector<QVariant> &args,
                            OpenSheet::ParseContext &ctx);
    static QVariant fnKurt(const QVector<QVariant> &args,
                            OpenSheet::ParseContext &ctx);
};
