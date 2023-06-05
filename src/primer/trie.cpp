#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // throw NotImplementedException("Trie::Get is not implemented.");

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.

  std::shared_ptr<const TrieNode> node = root_;

  return GetHp<T>(root_, key, 0);
}

template <class T>
auto Trie::GetHp(std::shared_ptr<const TrieNode> node, std::string_view key, std::string_view::size_type index) const
    -> const T * {
  // throw NotImplementedException("Trie::Get is not implemented.");

  if (node == nullptr) return nullptr;
  if (index >= key.size()) {
    const TrieNodeWithValue<T> *res = dynamic_cast<const TrieNodeWithValue<T> *>(node.get());//父类转子类

    if (res == nullptr) return nullptr;//没找到，返回nullptr

    return res->value_.get();
  }

  //找下一个节点
  char k = key.at(index);
  if (node->children_.find(k) != node->children_.end()) return GetHp<T>(node->children_.at(k), key, index + 1);

  return nullptr;
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  auto trie = Trie();
  // auto k=std::move(value);

  //先把no-copy 的value交给shared_ptr.
  std::shared_ptr<T> v = std::make_shared<T>(std::move(value));
  // std::shared_ptr<T> v(&value);
  trie.root_ = PutHp(root_, key, v, 0);

  return trie;
  //  if (std::is_same<T,std::unique_ptr<uint32_t>>::value) {
  //
  //   return PutHp(std::make_shared<const TrieNode>(TrieNode()),root_,key,std::move(value),0);
  //  }
  //  else {
  //
  //    auto v=value;
  //    return PutHp(std::make_shared<const TrieNode>(TrieNode()),root_,key,v,0);
  //  }
}

template <class T>
auto PutHp(std::shared_ptr<const TrieNode> n_old, std::string_view key, std::shared_ptr<T> value,
            std::string_view::size_type index) -> std::shared_ptr<const TrieNode> {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  if (index == key.size()) {
    std::shared_ptr<const TrieNodeWithValue<T>> new_value = nullptr;
    if (n_old == nullptr)
      new_value = std::make_shared<const TrieNodeWithValue<T>>(value);
    else
      new_value = std::make_shared<const TrieNodeWithValue<T>>(n_old->children_, value);
    return new_value;
  }

  std::unique_ptr<TrieNode> new_value = nullptr;
  char k = key.at(index);
  if (n_old == nullptr) {
    new_value = std::make_unique<TrieNode>();

    (new_value->children_)[k] = PutHp<T>(nullptr, key, value, index + 1);

  } else {
    new_value = n_old->Clone();

    std::shared_ptr<const TrieNode> child = nullptr;

    if (n_old->children_.find(k) != n_old->children_.end()) child = n_old->children_.at(k);
    new_value->children_[k] = PutHp(child, key, value, index + 1);
  }

  return std::move(new_value);

  //      for(){
  //
  //      }
}

auto Trie::Remove(std::string_view key) const -> Trie {
  // throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.

  auto trie = Trie();
  // auto k=std::move(value);

  // std::shared_ptr<T> v(&value);
  trie.root_ = RemoveHp(root_, key, 0);

  return trie;
}

auto RemoveHp(std::shared_ptr<const TrieNode> n_old, std::string_view key, std::string_view::size_type index)
    -> std::shared_ptr<const TrieNode> {
  // throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.

  if (n_old == nullptr) return nullptr;

  if (index == key.size()) {      //找到了目标节点

    //节点没有任何的child,直接删除
    if (n_old->children_.size() == 0) return nullptr;

    //有child,降级为普通TrieNode，承接下方的children
    std::shared_ptr<const TrieNode> new_value = std::make_shared<const TrieNode>(n_old->children_);
    return new_value;
  }

  char k = key.at(index);
  std::unique_ptr<TrieNode> new_value = n_old->Clone();//直接clone一份新的节点。和原节点共享children

  std::shared_ptr<const TrieNode> child = nullptr;

  if (n_old->children_.find(k) != n_old->children_.end()) child = n_old->children_.at(k);//找到下一个遍历到的节点。
  std::shared_ptr<const TrieNode> sub = RemoveHp(child, key, index + 1);

  if (sub != nullptr)
    new_value->children_[k] = sub;//sub是被修改过的child节点。要将new_value对应的路径指向该节点。
  else if (child != nullptr)
    new_value->children_.erase(new_value->children_.find(k));//sub被删除了,需要将new_value对应位置清空。

  if (new_value->children_.size() == 0 && (!new_value->is_value_node_)) return nullptr;//new_value不含任何的sub,且new_value本身
                                                                                       //不承载value,应当被废弃删除。

  return std::move(new_value);
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// self-defined functions
// template auto Trie::GetHp(std::shared_ptr<const TrieNode> node,std::string_view key,std::string_view::size_type
// index) const -> const uint32_t *; template auto Trie::GetHp(std::shared_ptr<const TrieNode> node,std::string_view
// key,std::string_view::size_type index) const -> const uint64_t *; template auto Trie::GetHp(std::shared_ptr<const
// TrieNode> node,std::string_view key,std::string_view::size_type index) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

// template auto Trie::GetHp(std::shared_ptr<const TrieNode> node,std::string_view key,std::string_view::size_type
// index) const -> const Integer *; template auto Trie::GetHp(std::shared_ptr<const TrieNode> node,std::string_view
// key,std::string_view::size_type index) const -> const MoveBlocked *;

// template auto  Trie::PutHp(std::shared_ptr<const TrieNode> n_new,std::shared_ptr<const TrieNode> n_old,
//                            std::string_view key, Integer * value,std::string_view::size_type index) const -> Trie;
//
// template auto  Trie::PutHp(std::shared_ptr<const TrieNode> n_new,std::shared_ptr<const TrieNode> n_old,
//                   std::string_view key, MoveBlocked * value,std::string_view::size_type index) const -> Trie;

}  // namespace bustub
