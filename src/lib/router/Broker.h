#pragma once

#include "Node.h"
#include <memory>

namespace keryx {

class Broker : public INode {
   Broker(NodeID);
   ~Broker();

   // Network topology changes
   void set_parent(NodeID, INode &);
   void clear_parent();
   void add_child(NodeID, INode &);
   void remove_child(NodeID);
   void add_sibling(NodeID, INode &);
   void remove_sibling(NodeID);

   // Local activity
   StreamID add_local_stream(StreamName const &, IStreamDesc &,
                             std::vector<EventPtr> &&);
   void remove_local_stream(StreamID);
   void local_notify(StreamID, EventPtr &&);
   SubID add_local_subscription(StreamFilter, EventHandler);
   void remove_local_subscription(SubID);

   // From INode
   void advertise_remote_stream(NodeID, StreamID, StreamName, IStreamDesc &) override;
   void unadvertise_remote_stream(StreamID) override;
   void add_remote_subscription(NodeID,StreamID, SubID ) override;
   void remove_remote_subscription(NodeID,StreamID) override;
   void remote_notify(StreamID,SubID,SerializedNotification &) override;


private:
   struct Stream;
   struct Neighbour;
   struct LocalSubscription;
   enum class NeighbourKind;
   struct PImpl;
   std::unique_ptr<PImpl> me;

   void add_neighbour(std::unique_ptr<Neighbour> &&);
};

}
