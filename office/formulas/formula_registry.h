#pragma once
#include "../engine/formula_parser.h"
#include <QObject>

namespace OpenSheet {

/**
 * Registers all built-in formulas with a FormulaParser.
 * Also provides a searchable catalog used by the Insert Function dialog.
 */
class FormulaRegistry : public QObject {
    Q_OBJECT
public:
    struct FunctionMeta {
        QString name;
        QString category;       // Math, Text, Date, Logical, Lookup, Statistical
        QString syntax;         // e.g. "SUM(number1, [number2], …)"
        QString description;
        QStringList argNames;
        QStringList argDescriptions;
    };

    explicit FormulaRegistry(QObject *parent = nullptr);

    // Register all built-ins into a parser instance
    void registerAll(FormulaParser *parser) const;

    // Catalog access
    QVector<FunctionMeta>  allFunctions()                           const;
    QVector<FunctionMeta>  byCategory(const QString &category)     const;
    QVector<FunctionMeta>  search(const QString &query)            const;
    const FunctionMeta    *find(const QString &name)               const;
    QStringList            categories()                             const;

private:
    QVector<FunctionMeta> m_catalog;
    void buildCatalog();
};

} // namespace OpenSheet
