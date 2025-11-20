# open_tags = ["div", "p", "a"]
# self_tags = ["input", "image", "hr", "br"]
# function_tags = ["loop", "map"]
# attributes = ["style", "id", "key"]

# tags = []
# location = IN_CONTENT

# found = advance_to("<@(>>)")
# if found:
#   while true:
#     found = 
#       (location = IN_CONTENT and 
#            match("<" + open_tags + "@[/> ]@(> )", (tag){res = [tag,waiting_for_props_or_close]})
#         or match("<" + self_tags + "@[/ ]@(> )", (tag){res = [tag,waiting_for_props_or_close]})
#         or (tags.length > 0 
#             and match("</" + tags.top() + ">@(>>)", (tag){res = [null,tag_end]})))
#        or (location = IN_TAG and
#            match("@(spaces)" + attributes + "@[/> ]@(> )", (tag){res = [attribute,solo_attribute]})
#         or match("@(spaces)" + attributes + "=@(>>)", (tag){res = [attribute,waiting_for_attribute_value]})
#         or match("@(spaces)>@(>>)", (tag){res = [null,waiting_for_children_or_end]})
#         or match("@(spaces)/>@(>>)", (tag){res = [null,tag_end]})

#     if found:
#       switch res[1]:
#       case tag_end:
#         if tags.length > 1:
#           tags[-1].children.push(make_tag(tags.top()))
#           tags.pop()
#         else
#           return make_tag(tags.top())
#       case waiting_for_props_or_close:
#         tags.push(res[0])
#         location = IN_TAG;
#       case solo_attribute:
#         attributes.push(name=res[0], value="true")
#       case waiting_for_attribute_value:
        
#       case waiting_for_children_or_end:
#         tags.push(res[0])
      

#   (name:="This is a name")

#   =>

#   volt::attr::name("This is a name")

#   <div>
#     <div>(istrue ? <hr/> : </>)</div>
#   </div>
#   <div style="color:white" id=("id" + to_string(10)) onClick=[](event){...}>
#     <h1>Hello</h1>
#     <h1>("Hello 2")</h1>
#     <br/>
#     <p>{return "Hello 3";}</p>
#     <div>{
#       vector<VNodeHandle> nodes = { <hr/>, <hr/> }
#       return <>nodes</>;
#     }</div>
#     <loop(items.size(), [](index){
#       return <h2>(std::to_string(items[index]))</h2>
#     })/>
#     <map(items, [](item){
#       return <h2>(std::to_string(item))</h2>
#     })/>
#     {
#       vector<VNodeHandle> nodes;
#       for(auto& item: items) {
#         nodes.push_back(<h2 key=item.id>(std::to_string(item))</h2>);
#       }
#       return <>nodes</>;
#     }
#     MyComponent(this).<render(x, y, z)/>
#   </div>

#   volt::tag::div({ volt::attr::style("color:white") }, volt::tag::h1("Hello").p(12), volt::tag::br().p(13)).p(14)



#   MyComponent(this).<render(x, y, x)/>

#   =>

#   MyComponent(this).render(x,y,z).p(15)

p = 0
text = 'anything works <p a="hello"></div> more text </div> end'
def match(variants, expresion, result):
    global p, text
    pp = p
    for variant in variants:
        e = expresion.replace("@{}", variant)
        ep = 0
        while ep >= 0:
            if ep >= len(e):
                return [variant, result]
            if e[ep:].startswith("@(anything)"):
                ep += len("@(anything)")
                pos = e.find("@", ep)
                if pos == -1:
                    pos = len(e)
                start = e[ep:pos]
                print("START:", start)
                pp = text.find(start, pp)
                if pp >= 0:
                    ep = pos
            elif e[ep:].startswith("@(spaces)"):
                ep += len("@(spaces)")
                while pp < len(text) and text[pp].isspace():
                    pp += 1
            elif e[ep:].startswith("@(>>)" ):
                p = pp
                return [variant, result]
            elif e[ep:].startswith("@(> )" ):
                if text[pp-1] == ' ':
                    p = pp
                else:
                    p = pp - 1
                return [variant, result]
            elif e[ep:].startswith("@["):
                ep += 2
                pos = e.find("]", ep)
                chars = list(e[ep:pos])
                matched = False
                for char in chars:
                    
                    if text.startswith(char, pp):
                        ep = pos + 2
                        pp += 1
                        matched = True
                        break
                    else:
                        ep = ep + 1
                if not matched:
                    ep = -1
                    break
                ep = pos + 2

            if text.find(e, p) == p:
                p += len(e)
                return [variant, result]
    return [None]

res = match(["div", "p", "a"], "@(anything)<@{} a=\"hello\">", "tag_found")
print(res)