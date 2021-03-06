#include "sidepanel_monitor.h"
#include "ui_sidepanel_monitor.h"
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>
#include <QDebug>

#include "utils.h"
#include "BT_logger_generated.h"

SidepanelMonitor::SidepanelMonitor(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SidepanelMonitor),
    _zmq_context(1),
    _zmq_subscriber(_zmq_context, ZMQ_SUB),
    _connected(false),
    _msg_count(0)
{
    ui->setupUi(this);
    _timer = new QTimer(this);

    connect( _timer, &QTimer::timeout, this, &SidepanelMonitor::on_timer );
}

SidepanelMonitor::~SidepanelMonitor()
{
    delete ui;
}

void SidepanelMonitor::clear()
{
    if( _connected ) this->on_Connect();
}

void SidepanelMonitor::on_timer()
{
    if( !_connected ) return;

    zmq::message_t msg;
    try{
        while(  _zmq_subscriber.recv(&msg) )
        {
            _msg_count++;
            ui->labelCount->setText( QString("Messages received: %1").arg(_msg_count) );

            const char* buffer = reinterpret_cast<const char*>(msg.data());

            const uint32_t header_size = flatbuffers::ReadScalar<uint32_t>( buffer );
            const uint32_t num_transitions = flatbuffers::ReadScalar<uint32_t>( &buffer[4+header_size] );

            for(size_t index = 4; index < header_size +4; index +=3 )
            {
                uint16_t uid = flatbuffers::ReadScalar<uint16_t>(&buffer[index]);
                AbstractTreeNode* node = _loaded_tree.nodeAtUID( uid );
                node->status = convert(flatbuffers::ReadScalar<BT_Serialization::Status>(&buffer[index+2] ));
            }

            std::unordered_map<int, NodeStatus> node_status;

            qDebug() << "--------";

            for(size_t t=0; t < num_transitions; t++)
            {
                size_t offset = 8 + header_size + 12*t;

                //const double t_sec  = flatbuffers::ReadScalar<uint32_t>( &buffer[index] );
                //const double t_usec = flatbuffers::ReadScalar<uint32_t>( &buffer[index+4] );
                //double timestamp = t_sec + t_usec* 0.000001;
                uint16_t uid = flatbuffers::ReadScalar<uint16_t>(&buffer[offset+8]);
                //NodeStatus prev_status = convert(flatbuffers::ReadScalar<BT_Serialization::Status>(&buffer[index+10] ));
                NodeStatus status      = convert(flatbuffers::ReadScalar<BT_Serialization::Status>(&buffer[offset+11] ));

                int index = _loaded_tree.UidToIndex(uid);
                _loaded_tree.nodeAtIndex(index)->status = status;
                node_status[index] = status;
                qDebug() << _loaded_tree.nodeAtIndex(index)->instance_name << " : " << toStr(status);
            }
            // update the graphic part

            emit changeNodeStyle( "BehaviorTree", node_status );
        }
    }
    catch( zmq::error_t& err)
    {
        qDebug() << "ZMQ receive failed ";
    }
}

bool SidepanelMonitor::getTreeFromServer()
{
    try{
        zmq::message_t request(0);
        zmq::message_t reply;

        zmq::socket_t  zmq_client( _zmq_context, ZMQ_REQ );
        zmq_client.connect( _connection_address_req.c_str() );

        int timeout_ms = 1000;
        zmq_client.setsockopt(ZMQ_RCVTIMEO,&timeout_ms, sizeof(int) );

        zmq_client.send(request);

        bool received = zmq_client.recv(&reply);
        if( ! received )
        {
            return false;
        }

        const char* buffer = reinterpret_cast<const char*>(reply.data());
        auto fb_behavior_tree = BT_Serialization::GetBehaviorTree( buffer );

        _loaded_tree = BuildTreeFromFlatbuffers( fb_behavior_tree );

        loadBehaviorTree( _loaded_tree, "BehaviorTree" );

        std::unordered_map<int, NodeStatus> node_status;

        qDebug() << "--------";

        for(size_t t=0; t < _loaded_tree.nodesCount(); t++)
        {
            node_status[t] = _loaded_tree.nodes()[t].status;
        }
        emit changeNodeStyle( "BehaviorTree", node_status );
    }
    catch( zmq::error_t& err)
    {
        qDebug() << "ZMQ client receive failed: " << err.what();
        return false;
    }
    return true;
}

void SidepanelMonitor::on_Connect()
{
    if( !_connected)
    {
        QString address = ui->lineEdit->text();
        if( address.isEmpty() )
        {
            address = ui->lineEdit->placeholderText();
            ui->lineEdit->setText(address);
        }
        bool failed = false;
        if( !address.isEmpty() )
        {
            _connection_address_pub = "tcp://" + address.toStdString() + std::string(":1666");
            _connection_address_req = "tcp://" + address.toStdString() + std::string(":1667");
            try{
                _zmq_subscriber.connect( _connection_address_pub.c_str() );

                int timeout_ms = 1;
                _zmq_subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
                _zmq_subscriber.setsockopt(ZMQ_RCVTIMEO,&timeout_ms, sizeof(int) );

                if( !getTreeFromServer() )
                {
                    failed = true;
                    _connected = false;
                }
            }
            catch(zmq::error_t& err)
            {
                failed = true;
            }
        }
        else {
            failed = true;
        }

        if( !failed )
        {
            _connected = true;
            ui->lineEdit->setDisabled(true);
            _timer->start(20);
            connectionUpdate(true);
        }
        else{
            QMessageBox::warning(this,
                                 tr("ZeroMQ connection"),
                                 tr("Was not able to connect to [%1]\n").arg(_connection_address_pub.c_str()),
                                 QMessageBox::Close);
        }
    }
    else{
        _connected = false;
        ui->lineEdit->setDisabled(false);
        _timer->stop();

        connectionUpdate(false);
    }
}
