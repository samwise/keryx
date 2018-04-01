#include<string>

namespace keryx {

template <class Tag,class ValueT>
class ValueWrapper {
public:
   typedef ValueT Value;

   ValueWrapper() : _v(){}

   explicit ValueWrapper(ValueT const&v) : _v(v) {}
   explicit ValueWrapper (ValueT &&v) : _v (std::move(v)) {}

   ValueWrapper(ValueWrapper const&v) : _v (v._v) {}
   ValueWrapper(ValueWrapper &&v) : _v (std::move(v._v)) {}

   ValueWrapper &operator=(ValueWrapper const&v) {_v = v._v;return *this;}
   ValueWrapper &operator=(ValueWrapper &&v) {_v = std::move(v._v);return *this;}
   
   bool operator==(ValueWrapper const&v) const {return _v == v._v;}
   bool operator!=(ValueWrapper const&v) const {return _v != v._v;}
   bool operator<(ValueWrapper const&v) const {return _v < v._v;}

   ValueT const& value() const {return _v;}

private:
   ValueT _v;
};

class SubIDTag {};
typedef ValueWrapper<SubIDTag,std::string> SubID;

class NodeIDTag {};
typedef ValueWrapper<NodeIDTag,std::string> NodeID;

class SubCookieTag {};
typedef ValueWrapper<SubCookieTag,uint32_t> SubCookie;

class MsgTag {};
typedef ValueWrapper<MsgTag,std::string> Msg;

class MsgPartTag {};
typedef ValueWrapper<MsgPartTag,std::string> MsgPart;

class StreamIDTag {};
typedef ValueWrapper<StreamIDTag,std::string> StreamID;

class NotificationChannelTag {};
typedef ValueWrapper<NotificationChannelTag,std::string> NotificationChannelID;

typedef uint32_t SeqNumber;
typedef uint32_t SeqNumber;
typedef uint32_t PartNumber;
typedef uint32_t TotalParts;

class Advertisement;
class NotificationChannel;

class INode {
public:
   virtual void notify(SubID const&,SubCookie const&,Msg const&msg) = 0;
   virtual void notify_loss(SubID const,SubCookie const&) = 0;
   virtual void advertise (StreamID const&,Advertisement const&adv) = 0;
   virtual void unadvertise (StreamID const&) = 0;
   virtual void ack_subscription(SubID const&,SubCookie const&) = 0;
   virtual void ack_unsubscription(SubID const&,SubCookie const&) = 0;
   virtual void init() = 0;
   virtual void finit() = 0;
};


class Router {
public:
   void notify(StreamID const&,Msg const&msg);
   void notify_loss(StreamID const&);

   void advertise (StreamID const&,Advertisement const&adv);
   void unadvertise (StreamID const&);

   void subscribe(NodeID const&, SubID const&,SubCookie const&);
   void unsubscribe(NodeID const&, SubID const&);

   void add_node(NodeID const&,INode &);
   void remove_node(NodeID const&);
};

class NotificationParser {
public:
   NotificationParser(Router &r,NotificationChannelID const&);
   void notify_part(StreamID const&, SeqNumber, PartNumber, TotalParts, MsgPart const&msg);
   void notify_part(StreamID const&, SeqNumber, PartNumber, TotalParts, MsgPart const&msg);
};


}

int main() {return 0;}


