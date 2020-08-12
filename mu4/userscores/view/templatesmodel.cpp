#include "templatesmodel.h"

#include "log.h"

using namespace mu::userscores;
using namespace mu::notation;

TemplatesModel::TemplatesModel(QObject* parent)
    : QObject(parent)
{

}

void TemplatesModel::load()
{
    RetVal<Templates> templates = repository()->templates();
    if (!templates.ret) {
        LOGE() << templates.ret.toString();
    }

    m_allTemplates = templates.val;
    m_visibleTemplates = m_allTemplates;

    for (const Template& templ: m_allTemplates) {
        m_visibleCategoriesTitles << templ.categoryTitle;
    }

    emit categoriesChanged();
    emit templatesChanged();
}

void TemplatesModel::apply()
{
    NOT_IMPLEMENTED;
}

QStringList TemplatesModel::categoriesTitles() const
{
    return m_visibleCategoriesTitles.toList();
}

QStringList TemplatesModel::templatesTitles() const
{
    QStringList titles;

    for (const Template& templ: m_visibleTemplates) {
        bool acceptedByCategory = templ.categoryTitle == categoriesTitles()[m_currentCategoryIndex];

        if (acceptedByCategory) {
            titles << templ.title;
        }
    }

    return titles;
}

void TemplatesModel::setCurrentCategory(int index)
{
    if (m_currentCategoryIndex == index) {
        return;
    }

    m_currentCategoryIndex = index;
    emit templatesChanged();
}

void TemplatesModel::setCurrentTemplate(int index)
{
    if (m_currentTemplateIndex == index) {
        return;
    }

    m_currentTemplateIndex = index;
}

void TemplatesModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    updateBySearch();
}

void TemplatesModel::updateBySearch()
{
    m_visibleTemplates.clear();
    m_visibleCategoriesTitles.clear();

    m_currentCategoryIndex = 0;
    m_currentTemplateIndex = 0;

    for (const Template& templ: m_allTemplates) {
        if (titleAccepted(templ.title) || titleAccepted(templ.categoryTitle)) {
            m_visibleTemplates << templ;
            m_visibleCategoriesTitles << templ.categoryTitle;
        }
    }

    emit categoriesChanged();
    emit templatesChanged();
}

bool TemplatesModel::titleAccepted(const QString& title) const
{
    if (m_searchText.isEmpty()) {
        return true;
    }

    return title.contains(m_searchText, Qt::CaseInsensitive);
}
