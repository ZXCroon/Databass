import time
import random
import argparse
import jinja2


TYPES = ('INT', 'FLOAT', 'CHAR', 'VARCHAR', 'DATE')
COMPOP = ('=', '<', '>', '<=', '>=', '<>')


def generate_str(max_len=10):
    l = random.randint(1, max_len)
    ret = ''.join([chr(random.randint(32, 127)) for _ in range(l)])
    return ret.replace('\\', '\\\\').replace('"', '\\"').replace('\'', '\\\'')


def generate_date():
    fmt = random.choice(('%Y-%m-%d', '%Y/%m/%d'))
    t = int(time.time())
    t = random.randint(0, t)
    return time.strftime(fmt, time.localtime(t))


def generate_identifier(max_len=10, min_len=4):
    l = random.randint(min_len, max_len)
    return ''.join([random.choice((chr(random.randint(ord('A'), ord('Z'))),
                                   chr(random.randint(ord('a'), ord('z'))))) for _ in range(l)])


def generate_range(max_digits=8):
    digits = random.randint(1, max_digits)
    a, b = [random.randint(10 ** (digits - 1), 10 ** digits - 1) for _ in range(2)]
    if a > b:
        a, b = b, a
    return a, b


def generate_column():
    ty = random.choice(TYPES)
    name = generate_identifier()
    ret = {'type': ty, 'name': name}
    if ty in ('CHAR', 'VARCHAR'):
        ret['len'] = random.randint(1, 10)
    if ty in ('INT', 'FLOAT'):
        ret['range'] = generate_range()
    return ret


def generate_table_info(max_columns=40):
    n_cols = random.randint(1, max_columns)
    table_info = []
    for col in range(n_cols):
        col_info = generate_column()
        table_info.append(col_info)
    return table_info, generate_identifier()


def generate_value(column):
    if column['type'] == 'INT':
        return (random.randint(*column['range'])) * random.choice((-1, 1))
    elif column['type'] == 'FLOAT':
        return (random.randint(*column['range']) + random.random()) * random.choice((-1, 1))
    elif column['type'] == 'CHAR' or column['type'] == 'VARCHAR':
        return generate_str(max_len=column['len'])
    elif column['type'] == 'DATE':
        return generate_date()
    else:
        raise


def generate_condition(table_info):
    col = random.choice(table_info)
    comp_op = random.choice(COMPOP)
    pivot = generate_value(col)
    return {'name': col['name'], 'comp_op': comp_op, 'pivot': pivot}


def generate_conditions(table_info, max_conditions=3):
    n = random.randint(0, max_conditions)
    return [generate_condition(table_info) for _ in range(n)]


def generate_insert(table_info, table_name):
    return {'command': 'insert', 'table': table_name, 'values': [generate_value(col) for col in table_info]}


def generate_delete(table_info, table_name):
    return {'command': 'delete', 'table': table_name, 'conditions': generate_conditions(table_info)}


def generate_update(table_info, table_name):
    col = random.choice(table_info)
    return {'command': 'update', 'table': table_name, 'conditions': generate_conditions(table_info),
            'update_column': col['name'], 'update_value': generate_value(col)}


def generate_select_count(table_info, table_name):
    return {'command': 'select count', 'table': table_name, 'conditions': generate_conditions(table_info, max_conditions=3)}


def generate_test(args, delete_ratio=0.01):
    commands = []
    db = generate_identifier()
    commands.append({'command': 'drop database', 'database': db})
    commands.append({'command': 'create database', 'database': db})
    commands.append({'command': 'use', 'database': db})

    tables = [generate_table_info(args.max_columns) for _ in range(args.n_tables)]
    for table in tables:
        commands.append({'command': 'create table', 'table': table})

    for _ in range(args.n_commands):
        if random.random() < delete_ratio:
            commands.append(generate_delete(*random.choice(tables)))
        else:
            commands.append(random.choice((generate_insert, generate_update))(*random.choice(tables)))

    for _ in range(args.n_counts):
        commands.append(generate_select_count(*random.choice(tables)))

    return commands


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=str)
    parser.add_argument('-n', dest='n_commands', type=int, default=100)
    parser.add_argument('--n_tables', type=int, default=2);
    parser.add_argument('--n_counts', type=int, default=3);
    parser.add_argument('--max_columns', type=int, default=40)
    args = parser.parse_args()

    commands = generate_test(args)
    tpl = jinja2.Template(open('test.tpl', 'r').read())
    open(args.output, 'w').write(tpl.render(commands=commands))
