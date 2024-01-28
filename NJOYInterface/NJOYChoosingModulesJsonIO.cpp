#include "NJOYChoosingModulesJsonIO.h"

NJOYChoosingModulesJsonIO::NJOYChoosingModulesJsonIO():
    moder(nullptr)
{

}

NJOYChoosingModulesJsonIO::NJOYChoosingModulesJsonIO(NJOYChoosingModulesJsonIO &&)
{

}

NJOYChoosingModulesJsonIO::~NJOYChoosingModulesJsonIO()
{

}

void NJOYChoosingModulesJsonIO::deleteBroadr()
{
    broadr.clear();
}

void NJOYChoosingModulesJsonIO::deleteGroupr()
{
    groupr.clear();
}

void NJOYChoosingModulesJsonIO::deleteReconr()
{
    reconr.clear();
}

void NJOYChoosingModulesJsonIO::deleteUnresr()
{
    unresr.clear();
}

std::unique_ptr<NJOYModerInput>NJOYChoosingModulesJsonIO::getModer()
{
    if (!moder)
        moder = std::make_unique<NJOYModerInput>();

    return std::move(moder);
}

void NJOYChoosingModulesJsonIO::setModer(std::unique_ptr<NJOYModerInput> newModer)
{
    moder = std::move(newModer);
}

std::vector<std::unique_ptr<NJOYBroadrInput> > &&NJOYChoosingModulesJsonIO::getBroadr()
{
    return std::move(broadr);
}

void NJOYChoosingModulesJsonIO::setBroadr(std::unique_ptr<NJOYBroadrInput> &&newBroadr)
{
    broadr.push_back(std::move(newBroadr));
}

std::vector<std::unique_ptr<NJOYGrouprInput> > &&NJOYChoosingModulesJsonIO::getGroupr()
{
    return std::move(groupr);
}

void NJOYChoosingModulesJsonIO::setGroupr(std::unique_ptr<NJOYGrouprInput> &&newGroupr)
{
    groupr.push_back(std::move(newGroupr));
}

std::vector<std::unique_ptr<NJOYReconrInput>> &&NJOYChoosingModulesJsonIO::getReconr()
{
    return std::move(reconr);
}
void NJOYChoosingModulesJsonIO::setReconr(std::unique_ptr<NJOYReconrInput> &&newReconr)
{
    reconr.push_back(std::move(newReconr));
}

std::vector<std::unique_ptr<NJOYUnresInput> > &&NJOYChoosingModulesJsonIO::getUnresr()
{
    return std::move(unresr);
}

void NJOYChoosingModulesJsonIO::setUnresr(std::unique_ptr<NJOYUnresInput> &&newUnresr)
{
    unresr.push_back(std::move(newUnresr));
}

NJOYModerInput::~NJOYModerInput()
{

}


