# Handle-based memory storage: first pass

This is the current direction for X+ data storage: typed slot pools, generational
handles, lightweight C++ views, and local change detection. Binary Front/Back
replication and clean-slate connection resets are described in
[Data services](DATA-SERVICES.md). Persistence and snapshot recovery are deferred.

## Using Student views

```cpp
#include <MemoryViews.hpp>
#include <examples/Student.hpp>

using namespace voltxp;
using namespace voltxp::examples;

auto& store = authoringMemoryStore();
auto serviceRoot = store.createRoot();
auto studentListHandle = store.createContainer(serviceRoot);
StudentList students(store, studentListHandle);

auto alice = students.add("Alice", 20, 80.0);
auto bob = students.add("Bob", 30, 90.0);
alice.age = 21;
++alice.age;
alice.grade += 2.5;
alice.age = bob.age;                 // copies the value, never rebinds age
int age = alice.age;
auto name = alice.name.get();

auto anotherView = students.at(alice.handle());
students.remove(alice.handle());
// anotherView.age.get() now throws: the removed handle is stale.

store.destroyRoot(serviceRoot);      // service-owned lifetime, not view lifetime
```

`Student` uses composition and has one container view plus three fields (64 bytes
on x64/MEMORY64). Each `Field<T>`, `ContainerView`, and list view stores one
64-bit handle plus a store pointer (16 bytes on these targets). There is one
Student type regardless of which store owns its data; no store tag appears in
application types. Copy construction binds another view; field assignment copies
the value, including across stores. Whole-Student assignment remains disabled
in the example to avoid mixing rebinding with field writes.

`List<Student>` knows only how to create/bind views and manage list membership.
The sample's `Student::initialize` allocates fields in a fixed order; its constructor
binds an existing container. The latter never allocates store data. If initialization
fails, the list removes the partially created item; its callback observes the ordered
creation/partial allocation/removal rather than an application transaction.

## Typed storage and handles

Separate pools hold `bool`, `int32_t`, `int64_t`, `float`, `double`, `std::string`,
and containers. A handle contains a 32-bit slot and 32-bit generation. Pool access
checks bounds, rejects generation zero, and compares generations. Free slots are
recycled using links in vacant slots. Release resets the value and immediately
advances the generation, invalidating the issued handle even before reuse. If
the generation cannot advance without wrapping, the slot is retired at generation
zero and never returned to the free list. Pool capacity follows allocation high-water marks rather than
total allocations, except for retired slots. Pools retain their capacity on release.

Slots hold `T` directly, without `optional<T>`, with an occupancy flag for slot-only replica validation. Release swaps
the value with a fresh `T{}` so owned string/container resources are released.
Replica installation checks vacancy; local reads check both occupancy and generation.
Generations are local and are never transmitted; see DATA-SERVICES.md for the wire protocol.

All scalar and container handles live in one membership structure, indexed by
`MemoryType`. Containers are members, not a separate attributes/children model.
`memberCount` and `memberAt` work uniformly for every member type.

There is one container representation and one `ContainerView`, without an object/list
flag, sealing, or an initialization phase. Any container can acquire scalar or
container members at any time. The application decides what those members mean.
`List<Student>` is an optional typed facade that initializes a Student's fields
when adding an item; it imposes no storage-level distinction.

Use `allocate<T>` to add scalar fields and `createContainer` (or
`ContainerView::add`) to add container members. Only whole containers can be
removed individually. There is no `RemoveValue` operation or scalar-removal API;
scalar fields are released with their containing container. Container removal
finds the child in the supplied parent membership in O(n), preserves order, and recursively
releases its contents. Destroying a root releases its entire tree. This first
pass assumes application-controlled tree depth.

Field names and indices exist only in view code, not in change records. The
Student constructor binds its first i32 member to age and first double member
to grade. View authors remain responsible for initializing that schema correctly.

Reads return values, not mutable references into reallocatable arrays. String reads
copy the string. Setters compare before recording: equal values create no event.
Repeated NaNs are treated as unchanged, while positive and negative floating-point
zero are distinct. Integer `+=`/increment reject signed overflow without changing
the value. Scalar pools store values directly, with generation, occupancy,
and free-list bookkeeping around them. There are no Cell wrappers or cached
parent/root handles. Containers store only their membership lists. The eight-byte
size applies to a handle, not a view or complete backing slot.

## Store identity and read-only access

Pass the store explicitly when binding views. The optional `authoringMemoryStore()`
singleton still exists, but fields do not depend on it. For example:

```cpp
MemoryStore front, back;
StudentList frontStudents(front, front.createRoot());
StudentList backStudents(back, back.createRoot());
auto local = frontStudents.add("Alice", 20);
auto remote = backStudents.add("Bob", 30);
local.age = remote.age; // same Student and Field types, different stores
```

`makeReadOnly()` irreversibly rejects local mutations of a store, including through
already-bound fields. Reads remain available. This moves enforcement from template
types to runtime checks, so callers can accept `const Student&` from either store.
The explicit `apply` and `restore` replication operations can update a read-only
store without generating outgoing changes. `apply` rejects author stores;
`restore` also supports resetting an author to the server's accepted reconnect state.

Stores must outlive all views pointing to them. They are non-copyable/non-movable
and root deletion retains generation history. Replica snapshot restoration replaces
pool state; rebind application views after restoration. A handle alone
is scoped to a store and pool. The single `Handle` type contains slot/generation;
`Field<T>` and `get<T>`/`set<T>` supply the scalar type. Do not mix raw handles
between stores or pools: matching numbers are not proof of the same identity.

Every operation runs on the owning thread. There is no implicit current-session
store. Authors can still choose small on-demand object views by storing one
`ContainerView` and returning `field<T>(index)` temporarily instead of holding
several `Field<T>` members.

The chosen server ownership model is one Front/Back store pair per session,
including server-authored Back data. The session owns its stores and change handlers;
its connection supplies network context. No root-routing metadata is needed
inside slots or change records. Session/network integration remains future work.

`destroyRoot(handle)` is explicitly a caller-owned root operation. It validates
that the handle is live, but cannot prove that it is a root: the store retains
no ancestry metadata or root registry. Calling it on a nested container would
leave a dangling membership in its owner. Use `removeItem(parent, child)` for
nested removal. `List::at(handle)` checks the parent's membership (O(n));
`ContainerView::atIndex` retrieves a member handle directly by position.

## Direct change detection

`setOnChange(std::function<void(MemoryChange)>)` installs the synchronous outgoing
handler. There is no journal inside `MemoryStore`. Completed mutations notify it:

- Allocate container: parent supplied by the caller and new handle.
- Allocate scalar: supplied parent, handle, scalar type and initial value.
- Set scalar: handle, scalar type and value; the parent field is empty.
- Remove container: supplied parent and handle; descendant removals are implicit.
  Root teardown uses an empty parent.

```cpp
store.setOnChange([](voltxp::MemoryChange change) {
    // Serialize/send this individual change here.
});
```

The handler sees the already-updated store, including completed deletion. Equal
assignments do not notify. Replica `apply` suppresses outgoing changes but calls the separate reader notification.
`setOnUpdated` observes author writes, replica apply, and resets. Legacy snapshot
`restore` remains silent and is not used for connection resets.
An empty handler is allowed for standalone/offline storage; it retains nothing.
Install another handler, or `{}` to remove it, outside callback execution.

Callbacks run on the owning thread and may read the store, but must not mutate it,
restore it, or replace its handler during notification. Exceptions propagate;
the mutation remains committed. Transport failures require disconnect/resync,
not local rollback or retry. The callback receives its record by value and may
move it into a serializer; references must not outlive the callback argument.

The [v1 data service](DATA-SERVICES.md) forwards this callback to its configured
sender immediately while connected.
