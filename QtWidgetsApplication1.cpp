#include "QtWidgetsApplication1.h"

#include "mz_compat.h"

#include <qlayout.h>
#include <QTableView>
#include <qheaderview.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <QMessageBox.h>
//#include <qexception.h>

class mz_wrapper
{
public:
    mz_wrapper(QString&& path): path_(qMove(path))
    {
    }

    ~mz_wrapper( )
    {
        close( );
    }

    mz_wrapper(const mz_wrapper&) = delete;
    auto operator=(const mz_wrapper&) -> mz_wrapper& = delete;

    mz_wrapper(mz_wrapper&& other) noexcept
    {
        *this = qMove(other);
    }

    auto operator=(mz_wrapper&& other) noexcept -> mz_wrapper&
    {
        this->close( );
        path_ = qMove(other.path_);
        mz_ = qMove(other.mz_);
        info_cached_ = qMove(other.info_cached_);

        other.mz_.file = nullptr;

        return *this;
    }

    auto open( ) -> void
    {
        const auto file = unzOpen(path_.toUtf8( ).constData( ));
        if (file == nullptr)
        {
            assert(0);
            throw std::exception("Unable to open file!");
        }
        mz_.file = file;
        if (unzGetGlobalInfo(mz_.file, &mz_.info) != MZ_OK)
        {
            assert(0);
            throw std::exception("Unable to get file info!");
        }
    }

    auto close( ) -> void
    {
        if (mz_.file == nullptr)
            return;
        unzClose(mz_.file);
        mz_ = { };
    }

#if 0
    struct simple_file_info
    {
        simple_file_info(std::string&& full_path, const unz_file_info& entry_info): path(qMove(full_path)), info(entry_info) { }
        simple_file_info(const std::string_view& full_path, const unz_file_info& entry_info): path((full_path)), info(entry_info) { }

        std::string path;
        unz_file_info info;
    };

    using file_info_storage = std::vector<simple_file_info>;
#endif

    //std::map �� ���� ������� �����������. ���������� ����� ������������� ��� �������
    //using file_info_storage_temp = std::map<QString, unz_file_info, std::greater<QString> >;
    using file_info_storage = QVector<QPair<QString, unz_file_info> >;

    auto update_file_info(bool force = false) -> const file_info_storage&
    {
        if (force || info_cached_.empty( ))
        {
            file_info_storage/*_temp*/ info;

            const auto last_entry = mz_.info.number_entry;
            const auto prev_last_entry = last_entry - 1u;
            for (decltype(mz_.info.number_entry) i = 0; i < last_entry; ++i)
            {
                // Get info about current file.
                unz_file_info zip_file_info;

#if 0
#if defined(unix) || defined(__unix__) || defined(__unix)
             char filename[NAME_MAX];
#else
                char filename[_MAX_FNAME];
#endif
#endif
                char filename[255];

                if (unzGetCurrentFileInfo(mz_.file, &zip_file_info, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK)
                {
                    assert(0);
                    throw std::exception("Unable to read file info!");
                }

                const auto filename_sv = std::string_view(filename);
                const auto filename_back = filename_sv.back( );
                if (filename_back != '\\' && filename_back != '/') //or use mz_path_has_slash
                {
                    //file detected

                    auto qstr = QString::fromUtf8(filename_sv.data( ), filename_sv.size( ));
                    file_info_storage::value_type info_data;//QPair have no rvalue constructor
                    info_data.first=qMove(qstr);
                    info_data.second=zip_file_info;
                    info.push_back(qMove(info_data));
                    //info.push_back(QPair(qMove(str), zip_file_info));
                }
                else
                {
                    //directory detected
                }

                if (i != prev_last_entry && unzGoToNextFile(mz_.file) != UNZ_OK)
                {
                    assert(0);
                    throw std::exception("Unable to get next archived file!");
                }
            }

            if (!info.empty( ) && unzGoToFirstFile(mz_.file) != UNZ_OK)
            {
                assert(0);
                throw std::exception("Unable to get first archived file!");
            }

            info_cached_.reserve(info.size( ));
            for (auto itr = info.begin( ); itr != info.end( ); ++itr)
            {
                auto& [fst, snd] = *itr;
                info_cached_.push_back(QPair(const_cast<QString&&>(fst), qMove(snd)));
            }

            //info_cached_.assign(std::make_move_iterator(info.begin( )), std::make_move_iterator(info.end( )));
        }

        return info_cached_;
    }

    auto mz_info( ) const -> const auto&
    {
        return mz_;
    }

    auto file_path( ) const -> QStringRef
    {
        return QStringRef(&path_);
    }

private:
    QString path_;
    file_info_storage info_cached_;

    struct
    {
        unzFile file = nullptr;
        unz_global_info info = { };
    } mz_;
};

class mz_visualizer final: public /*QTableView*/QAbstractTableModel
{
public:
    mz_visualizer(QWidget* parent): QAbstractTableModel(parent)
    {
        QAbstractTableModel::setHeaderData(0, Qt::Orientation::Horizontal, "file inside archive");
        QAbstractTableModel::setHeaderData(1, Qt::Orientation::Horizontal, "uncompressed size");
        QAbstractTableModel::setHeaderData(2, Qt::Orientation::Horizontal, "compressed size");
    }

    int rowCount(const QModelIndex& parent) const override
    {
        const auto result = zip_.isNull( ) ? 0 : zip_->update_file_info( ).size( );
        return result;
    }

    int columnCount(const QModelIndex&) const override
    {
        return 3;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
                case 0: return tr("file inside archive");
                case 1: return tr("uncompressed size");
                case 2: return tr("compressed size");
                default:
                    assert(0);
                    break;
            }
        }

        return QAbstractTableModel::headerData(section, orientation, role);
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        /*switch((Qt::ItemDataRole)role)
        {
            case Qt::DisplayRole: break;
            case Qt::DecorationRole: break;
            case Qt::EditRole: break;
            case Qt::ToolTipRole: break;
            case Qt::StatusTipRole: break;
            case Qt::WhatsThisRole: break;
            case Qt::FontRole: break;
            case Qt::TextAlignmentRole: break;
            case Qt::BackgroundColorRole: break;
            case Qt::BackgroundRole: break;
            case Qt::TextColorRole: break;
            case Qt::ForegroundRole: break;
            case Qt::CheckStateRole: break;
            case Qt::AccessibleTextRole: break;
            case Qt::AccessibleDescriptionRole: break;
            case Qt::SizeHintRole: break;
            case Qt::InitialSortOrderRole: break;
            case Qt::DisplayPropertyRole: break;
            case Qt::DecorationPropertyRole: break;
            case Qt::ToolTipPropertyRole: break;
            case Qt::StatusTipPropertyRole: break;
            case Qt::WhatsThisPropertyRole: break;
            case Qt::UserRole: break;
            default: ;
        }*/

        //auto role1 = static_cast<Qt::ItemDataRole>(role);

        if (role == Qt::DisplayRole)
        {
            if (!zip_.isNull( ))
            {
                auto& info = zip_->update_file_info( );
                assert(!info.empty());

                const auto& [text, data] = info[index.row( )];

                switch (index.column( ))
                {
                    case 0: return text;
                    case 1: return QString::number(data.uncompressed_size);
                    case 2: return QString::number(data.compressed_size);
                    default:
                        assert(0);
                        break;
                }
            }
        }

        return QVariant( );
    }

    void FillData( )
    {
        const auto parent = static_cast<QWidget*>(this->parent( ));
        auto file_path = QFileDialog::getOpenFileName(parent, tr("caption"), nullptr, tr("Zip archive files (*.zip)"));
        //todo: detect zip update time
        const auto update = zip_.isNull( ) || zip_->file_path( ) != file_path;
        if (!update)
            return;

        try
        {
            zip_.reset(new mz_wrapper(qMove(file_path)));
            zip_->open( );
            zip_->update_file_info( );
            zip_->close( );

            this->layoutChanged( );
            //parent->adjustSize( );//do nothing

            QWidget* window = nullptr;
            auto top = QApplication::topLevelWidgets( );
            assert(!top.empty());
            if (top.size( ) == 1)
                window = top[0];
            else
            {
                for (auto& w: top)
                {
                    if (!w->windowTitle( ).startsWith("Zip viewer: "))
                        continue;

                    window = (w);
                    break;
                }
            }

            window->setWindowTitle(tr("Zip viewer: %1").arg(zip_->file_path( )));
        }
        catch (const std::exception& ex)
        {
            zip_->close( );
            QMessageBox::warning(parent, "Warning", tr("Unable to load file! %1").arg(ex.what( )));
        }
    }

private:
    QScopedPointer<mz_wrapper> zip_;
};

QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    this->setWindowTitle(tr("Zip viewer: idle..."));

    auto table_view = new QTableView;
    const auto visualizer = new mz_visualizer(table_view/*this*/);

    table_view->setModel(visualizer);
    auto hheader = table_view->horizontalHeader( );
    auto vheader = table_view->verticalHeader( );
    //hheader->setDefaultAlignment(Qt::AlignCenter);
    //vheader->setDefaultAlignment(Qt::AlignCenter);
    hheader->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    vheader->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    auto layout = new QFormLayout/*QVBoxLayout*/;
    ui.centralWidget->setLayout(layout);

    const auto open_dialog_button = new QPushButton("Select zip file");
    connect(open_dialog_button, &QPushButton::released, visualizer, &mz_visualizer::FillData);
    //connect(visualizer, &mz_visualizer::FillData, table_view, &QTableView::adjustSize);

    layout->addWidget(open_dialog_button);
    layout->addWidget(table_view);

    table_view->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    //table_view->adjustSize( );
}
