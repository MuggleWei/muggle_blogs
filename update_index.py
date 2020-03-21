import logging
import os


def clear_index(path):
    index_path = os.path.join(path, "index.md")
    if os.path.exists(index_path):
        os.remove(path=index_path)


def gen_blog_tree(path):
    """
    generate blogs index
    :param path:
    :return:
    """
    blogs = []
    # if .md file in the folder that has same name, then it's blog
    for root, dirs, files in os.walk(path):
        for filename in files:
            vec = filename.split('.')
            if len(vec) >= 2 and vec[-1] == 'md' and vec[0] == os.path.basename(root):
                blogs.append(os.path.join(root, filename))

    index = {}
    for blog in blogs:
        blog = blog.replace('\\', '/')
        vec = blog.split('/')
        cur_level = index
        for v in vec:
            if v not in cur_level:
                cur_level[v] = {}
                cur_level = cur_level[v]
                continue
            cur_level = cur_level[v]
    return index


def gen_index(f, tree, level, parent):
    for k, v in tree.items():
        for i in range(level):
            f.write('\t')
        f.write('- ')
        if k.endswith('.md'):
            all_path = os.path.join(parent, k)
            basename = os.path.basename(k)
            f.write('[{}]'.format(basename))
            f.write('({})'.format(all_path.replace('\\', '/')))
            f.write('\n')
        else:
            f.write(k)
            f.write('\n')
            gen_index(f, v, level + 1, os.path.join(parent, k))


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    # remove old index
    clear_index(path='./')

    # find index
    blog_tree = gen_blog_tree(path='blogs')

    # generate index md
    with open('index.md', 'w', encoding='utf-8') as f:
        gen_index(f, blog_tree, 0, '')
