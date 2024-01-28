#pragma once

#include <QWizardPage>

struct NJOYChoosingIsotopes;

namespace Ui {
class NJOYChoosingIsotopesWidget;
}

class NJOYChoosingIsotopesWidget : public QWizardPage
{
    Q_OBJECT

public:
    explicit NJOYChoosingIsotopesWidget(QWidget *parent = nullptr);
    ~NJOYChoosingIsotopesWidget();

    std::unique_ptr<NJOYChoosingIsotopes> save();
    void load(std::unique_ptr<NJOYChoosingIsotopes> choosingIso);

    QStringList getFileNames() const;

private slots:
    void setEndfFileOption(int option);
    void changeOrNotTapesSaved();

private:
    Ui::NJOYChoosingIsotopesWidget *ui;

    enum Option
    {
        Yes,
        No
    };

    void openFile();

    void setButtonGroup();

    void setConnections();

    void setFileNameInLabel();

    void setToolTipForNextButton() const;

    void hideWidgets();

    void setLink();

    void startFadeEffect(QWidget *w, int fadeDuration);

    QStringList fileNames;

    // QWizardPage interface
public:
    virtual void initializePage() override;
    virtual bool validatePage() override;
    virtual bool isComplete() const override;
};


