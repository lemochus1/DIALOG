
def placeDictToDict(parent_map, key, map = dict()):
    if key not in parent_map:
        parent_map[key] = map
    return parent_map


def placeToDictOfLists(map, key, value):
    if key in map:
        map[key].append(value)
    else:
        map[key] = [value]
    return map


def addToList(target_list, list_or_value):
    if list_or_value is not None:
        if isinstance(list_or_value, list):
            target_list.extend(list_or_value)
        else:
            target_list.append(list_or_value)


def insertIfBelongs(target_list, value, group, begin=True):
    for part in group:
        if part in value:
            if begin:
                target_list.insert(0, value)
            else:
                target_list.append(value)
            return True
    return False
