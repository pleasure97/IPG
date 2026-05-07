import argparse
import jinja2
import DataFormatParser

def main():
    
    argument_parser = argparse.ArgumentParser(description = "DataFormatGenerator")
    argument_parser.add_argument('--path', type=str, default='', help='data path')
    argument_parser.add_argument('--output', type=str, default='TestDataFormat', help='output path')
    args = argument_parser.parse_args()

    parser = DataFormatParser()
    parser.parse_data_format(args.path)
    
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
    
    template = env.get_template('~.h')
    output = template.render(parser = parser)

    f = open(args.output+'.h', 'w+')
    f.write(output)
    f.close()

    return

if __name__ == '__main__':
    main()
