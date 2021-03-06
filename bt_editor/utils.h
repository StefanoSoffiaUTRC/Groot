#ifndef NODE_UTILS_H
#define NODE_UTILS_H

#include <nodes/NodeData>
#include <nodes/FlowScene>
#include <nodes/NodeStyle>

#include "bt_editor_base.h"
#include "BT_logger_generated.h"

namespace tinyxml2{
    class XMLElement;
}

QtNodes::Node* findRoot(const QtNodes::FlowScene &scene);

std::vector<QtNodes::Node *> getChildren(const QtNodes::FlowScene &scene,
                                         const QtNodes::Node &parent_node,
                                         bool ordered);

AbsBehaviorTree BuildTreeFromScene(const QtNodes::FlowScene *scene);

AbsBehaviorTree BuildTreeFromFlatbuffers(const BT_Serialization::BehaviorTree* bt );

AbsBehaviorTree BuildTreeFromXML(const tinyxml2::XMLElement* bt_root);

void NodeReorder(QtNodes::FlowScene &scene, AbsBehaviorTree &abstract_tree );

QString getCategory(const QtNodes::NodeDataModel* model);

std::pair<QtNodes::NodeStyle, QtNodes::ConnectionStyle>
 getStyleFromStatus(NodeStatus status);

inline NodeType convert(const BT_Serialization::Type& type)
{
    switch( type )
    {
    case BT_Serialization::Type::ACTION    : return  NodeType::ACTION;
    case BT_Serialization::Type::DECORATOR : return  NodeType::DECORATOR;
    case BT_Serialization::Type::CONTROL   : return  NodeType::CONTROL;
    case BT_Serialization::Type::SUBTREE   : return  NodeType::SUBTREE;
    case BT_Serialization::Type::CONDITION : return  NodeType::CONDITION;
    case BT_Serialization::Type::UNDEFINED : return  NodeType::UNDEFINED;
    }
    return NodeType::UNDEFINED;
}

inline NodeStatus convert(const BT_Serialization::Status& status)
{
    switch( status )
    {
    case BT_Serialization::Status::IDLE    : return NodeStatus::IDLE;
    case BT_Serialization::Status::RUNNING : return NodeStatus::RUNNING;
    case BT_Serialization::Status::SUCCESS : return NodeStatus::SUCCESS;
    case BT_Serialization::Status::FAILURE : return NodeStatus::FAILURE;
    }
    return NodeStatus::IDLE;
}

const std::set<std::string>& BuiltInRegisteredModels();

ParameterWidgetCreator buildWidgetCreator(const TreeNodeModel::Param& param);

bool addToModelRegistry(QtNodes::DataModelRegistry& registry,
                   const QString& ID,
                   const ParameterWidgetCreators& parameters,
                   NodeType node_type);

#endif // NODE_UTILS_H
