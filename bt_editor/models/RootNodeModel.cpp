#include "RootNodeModel.hpp"
#include <QLineEdit>
#include <QDebug>

RootNodeModel::RootNodeModel():
    BehaviorTreeDataModel ( RootNodeModel::Name(),
                            RootNodeModel::Name(),
                            ParameterWidgetCreators() )
{
    _line_edit_name->setHidden(true);
}

unsigned int RootNodeModel::nPorts(QtNodes::PortType portType) const
{
    return (portType==PortType::In) ? 0:1;
}


NodeDataModel::ConnectionPolicy RootNodeModel::portOutConnectionPolicy(QtNodes::PortIndex) const
{
    return ConnectionPolicy::One;
}
