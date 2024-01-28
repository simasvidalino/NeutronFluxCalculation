#include "NJOYModerInputDlg.h"
#include "ui_NJOYModerInputDlg.h"

#include "NJOYChoosingModulesJsonIO.h"

NJOYModerInputDlg::NJOYModerInputDlg(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYModerInputDlg)
{
    ui->setupUi(this);

    setConnections();
}

NJOYModerInputDlg::~NJOYModerInputDlg()
{
    delete ui;
}

std::unique_ptr<NJOYModerInput> NJOYModerInputDlg::save()
{
    auto moder = std::make_unique<NJOYModerInput>();

    moder->input   = std::to_string(ui->spinBoxInput->value());
    moder->output  = std::to_string(ui->spinBoxOutput->value());

    if (ui->spinBoxInput->value() == 1)
    {
        std::map<std::string, std::string> map;

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            auto file = ui->tableWidget->item(row, 0)->text().toStdString();
            auto mat  = ui->tableWidget->item(row, 1)->text().toStdString();

            map[file] = mat;
        }

        moder->inputTapesAndMat.reset();
        moder->inputTapesAndMat = map;
    }

    return moder;
}

void NJOYModerInputDlg::load(std::unique_ptr<NJOYModerInput> moder)
{
    if (!moder)
        moder = std::make_unique<NJOYModerInput>();

    std::cout<<"moder "<<moder->input<<" "<<moder->output<<std::endl;

    ui->spinBoxInput->setValue(QString::fromStdString(moder->input).toInt());
    ui->spinBoxOutput->setValue(QString::fromStdString(moder->output).toInt());

    bool hasTapesAndMat = moder->inputTapesAndMat.has_value();
    ui->tableWidget->setVisible(hasTapesAndMat );
    ui->labelTextTapeAndMat->setVisible(hasTapesAndMat);

    if (hasTapesAndMat)
    {
        auto inputTapesAndMatIt = moder->inputTapesAndMat.value().begin();
        int row = moder->inputTapesAndMat.value().size();

        ui->tableWidget->setRowCount(row);

        for (int r = 0; r < row; ++r)
        {
            auto tape = QString::fromStdString(inputTapesAndMatIt->first);
            auto mat  = QString::fromStdString(inputTapesAndMatIt->second);

            auto tapeItem = ui->tableWidget->item(r, 0);
            auto matItem = ui->tableWidget->item(r, 1);

            if (!tapeItem)
            {
                auto tapeItem = new QTableWidgetItem(tape);
                auto matItem  = new QTableWidgetItem(mat);

                tapeItem->setFlags(tapeItem->flags() & ~Qt::ItemIsEditable);

                ui->tableWidget->setItem(r, 0, tapeItem);
                ui->tableWidget->setItem(r, 1, matItem);
            }
            else
            {
                tapeItem->setText(tape);
                matItem->setText(mat);

                tapeItem->setFlags(tapeItem->flags() & ~Qt::ItemIsEditable);
            }

            ++inputTapesAndMatIt;
        }
    }

    this->setMaximumHeight(this->sizeHint().height());
}

void NJOYModerInputDlg::onMergeFiles(bool checked)
{
    // ui->plainTextTapeAndMat->setVisible(checked);
    ui->labelTextTapeAndMat->setVisible(checked);
}

void NJOYModerInputDlg::setConnections()
{
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
