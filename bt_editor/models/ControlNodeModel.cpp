#include "ControlNodeModel.hpp"
#include "ControlNodeModel.hpp"
#include <QtDebug>

ControlNodeModel::ControlNodeModel(const QString &ID, const ParameterWidgetCreators &parameters ):
    BehaviorTreeDataModel("Control", ID , parameters)
{
    _line_edit_name->setEnabled(true);
    _line_edit_name->setReadOnly(false);

    _line_edit_name->setStyleSheet("color: rgb(30,30,30); "
                                   "background-color: rgb(180,180,180);"
                                   "border: 0px;");
}


SequenceModel::SequenceModel(): ControlNodeModelBase()
{

}

FallbackModel::FallbackModel(): ControlNodeModelBase()  {

}

SequenceStarModel::SequenceStarModel(): ControlNodeModelBase() {
    updateNodeSize();
}

IfThenElseModel::IfThenElseModel(): ControlNodeModelBase() {

}

