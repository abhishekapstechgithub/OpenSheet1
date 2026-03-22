#include "stats_plugin.h"
#include <cmath>
#include <numeric>
#include <algorithm>

bool StatsPlugin::initialize(OpenSheet::PluginContext *ctx)
{
    ctx->registerFormula("GEOMEAN", fnGeoMean);
    ctx->registerFormula("HARMEAN", fnHarMean);
    ctx->registerFormula("SKEW",    fnSkew);
    ctx->registerFormula("KURT",    fnKurt);
    ctx->log("StatsPlugin: registered GEOMEAN, HARMEAN, SKEW, KURT");
    return true;
}

void StatsPlugin::shutdown()
{
    // Nothing to clean up
}

QVariant StatsPlugin::fnGeoMean(const QVector<QVariant> &args,
                                 OpenSheet::ParseContext &)
{
    if (args.isEmpty()) return 0.0;
    double logSum = 0.0;
    int n = 0;
    for (const auto &a : args) {
        bool ok; double v = a.toDouble(&ok);
        if (ok && v > 0) { logSum += std::log(v); ++n; }
    }
    return n > 0 ? std::exp(logSum / n) : 0.0;
}

QVariant StatsPlugin::fnHarMean(const QVector<QVariant> &args,
                                 OpenSheet::ParseContext &)
{
    if (args.isEmpty()) return 0.0;
    double recipSum = 0.0;
    int n = 0;
    for (const auto &a : args) {
        bool ok; double v = a.toDouble(&ok);
        if (ok && !qFuzzyIsNull(v)) { recipSum += 1.0 / v; ++n; }
    }
    return (n > 0 && !qFuzzyIsNull(recipSum)) ? n / recipSum : 0.0;
}

QVariant StatsPlugin::fnSkew(const QVector<QVariant> &args,
                              OpenSheet::ParseContext &)
{
    QVector<double> vals;
    for (const auto &a : args) { bool ok; double v = a.toDouble(&ok); if(ok) vals.append(v); }
    int n = vals.size();
    if (n < 3) return 0.0;

    double mean = std::accumulate(vals.begin(), vals.end(), 0.0) / n;
    double m2 = 0, m3 = 0;
    for (double v : vals) {
        double d = v - mean;
        m2 += d * d;
        m3 += d * d * d;
    }
    m2 /= n; m3 /= n;
    double sd = std::sqrt(m2);
    if (qFuzzyIsNull(sd)) return 0.0;
    return (static_cast<double>(n) / ((n-1.0) * (n-2.0))) * (m3 / std::pow(sd, 3));
}

QVariant StatsPlugin::fnKurt(const QVector<QVariant> &args,
                              OpenSheet::ParseContext &)
{
    QVector<double> vals;
    for (const auto &a : args) { bool ok; double v = a.toDouble(&ok); if(ok) vals.append(v); }
    int n = vals.size();
    if (n < 4) return 0.0;

    double mean = std::accumulate(vals.begin(), vals.end(), 0.0) / n;
    double m2 = 0, m4 = 0;
    for (double v : vals) {
        double d = v - mean;
        m2 += d * d;
        m4 += d * d * d * d;
    }
    m2 /= n; m4 /= n;
    if (qFuzzyIsNull(m2)) return 0.0;
    // Fisher's kurtosis (excess)
    double kurt = m4 / (m2 * m2) - 3.0;
    // Sample correction
    double corr = ((double)n*(n+1)) / ((n-1.0)*(n-2.0)*(n-3.0));
    double bias = 3.0 * (n-1.0) * (n-1.0) / ((n-2.0) * (n-3.0));
    return corr * (m4 / (m2 * m2)) * n - bias;
}
